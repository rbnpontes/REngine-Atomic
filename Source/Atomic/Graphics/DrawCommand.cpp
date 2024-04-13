#include "./DrawCommand.h"

#include "./Geometry.h"
#include "./Model.h"
#include "./Renderer.h"
#include "./Shader.h"
#include "./Texture2D.h"
#include "./TextureCube.h"
#include "./VertexBuffer.h"
#include "./IndexBuffer.h"
#include "./ShaderVariation.h"
#include "./ShaderProgram.h"
#include "./RenderTexture.h"

#include "../RHI/RenderCommand.h"
#include "../RHI/PipelineStateBuilder.h"
#include "../Core/Profiler.h"
#include "../IO/Log.h"
#include "../RHI/GraphicsState.h"
#include "../RHI/DiligentUtils.h"

namespace REngine
{
	using namespace Atomic;

	static ea::array<Diligent::ITextureView*, MAX_RENDERTARGETS> s_render_targets = {};

	static Diligent::ITextureView* s_depth_stencil = nullptr;
	static u8 s_num_rts = 0;

	static ea::array<Diligent::IBuffer*, MAX_VERTEX_STREAMS> s_vertex_buffers = {};
	static u32 s_vertex_buffer_hash = 0;
	static Diligent::IBuffer* s_index_buffer = nullptr;

	static ea::queue<ShaderParameterUpdateDesc> s_empty_params_queue = {};

	class DrawCommandImpl : public IDrawCommand
	{
	public:
		DrawCommandImpl(Graphics* graphics, Diligent::IDeviceContext* context) :
			context_(context),
			graphics_(graphics),
			viewport_(IntRect::ZERO),
			dirty_flags_(static_cast<u32>(RenderCommandDirtyState::all)),
			enable_clip_planes_(false),
			curr_pipeline_hash_(0),
			curr_vertx_decl_hash_(0),
			curr_vertex_buffer_hash_(0),
			num_batches_(0),
			primitive_count_(0),
			vertex_buffers_({}),
			vertex_offsets_({}),
			params_2_update_({}),
			shader_param_sources_({})

		{
			pipeline_info_ = new PipelineStateInfo();
		}
		~DrawCommandImpl() override
		{
			// TODO: clear static states too
			delete pipeline_info_;
		}

		void Reset() override
		{
			*pipeline_info_ = PipelineStateInfo{};
			
			const auto wnd_size = graphics_->GetRenderTargetDimensions();
			viewport_ = IntRect(0, 0, wnd_size.x_, wnd_size.y_);

			pipeline_state_ = nullptr;
			vertex_declaration_ = nullptr;
			shader_program_ = nullptr;
			depth_stencil_ = nullptr;
			index_buffer_ = nullptr;
			shader_resource_binding_ = nullptr;

			index_type_ = ValueType::VT_UNDEFINED;

			params_2_update_ = {};
			shader_param_sources_.fill(M_MAX_UNSIGNED);

			render_targets_.fill(nullptr);
			textures_.fill(ShaderResourceTextureDesc{});
			vertex_buffers_.fill(nullptr);
			vertex_offsets_.fill(0);

			enable_clip_planes_ = false;
			clip_plane_ = Vector4::ZERO;
			curr_pipeline_hash_ = curr_vertex_buffer_hash_ = curr_vertx_decl_hash_ = 0u;

			dirty_flags_ = static_cast<u32>(RenderCommandDirtyState::all);
			num_batches_ = 0;
			primitive_count_ = 0;

			dirty_flags_ = static_cast<u32>(RenderCommandDirtyState::all);
		}
		void Clear(const DrawCommandClearDesc& desc) override
		{
			ATOMIC_PROFILE(IDrawCommand::Clear);
			const auto rt_size = graphics_->GetRenderTargetDimensions();

			if(!viewport_.left_ && !viewport_.top_ && viewport_.right_ == rt_size.x_ && viewport_.bottom_ == rt_size.y_)
			{
				const auto old_depth_write = pipeline_info_->depth_write_enabled;
				const auto old_dirty_flags = dirty_flags_;

				pipeline_info_->depth_write_enabled = true;
				PrepareClear();

				pipeline_info_->depth_write_enabled = old_depth_write;
				// if previous dirty flags does not change pipeline. then we must remove pipeline from dirty flags
				if(old_dirty_flags & static_cast<u32>(RenderCommandDirtyState::pipeline) == 0)
					dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::pipeline);

				ClearByHardware(desc);
				return;
			}

			PrepareClear();
			ClearBySoftware(desc);
		}
		void Draw(const DrawCommandDrawDesc& desc) override
		{
			ATOMIC_PROFILE(IDrawCommand::Draw);

			PrepareDraw();

			if(index_buffer_)
			{
				context_->DrawIndexed({
					desc.index_count,
					index_type_,
					DRAW_FLAG_NONE,
					1,
					desc.index_start,
					desc.base_vertex_index
				});
			}
			else
			{
				context_->Draw({
					desc.vertex_count,
					DRAW_FLAG_NONE,
					1,
					desc.vertex_start
				});
			}

			u32 primitive_count;
			utils_get_primitive_type(desc.vertex_count, pipeline_info_->primitive_type, &primitive_count);
			primitive_count_ += primitive_count;
			++num_batches_;
		}
		void Draw(const DrawCommandInstancedDrawDesc& desc) override
		{
			ATOMIC_PROFILE(IDrawCommand::Draw);

			PrepareDraw();

			context_->DrawIndexed({
				desc.index_count,
				index_type_,
				DRAW_FLAG_NONE,
				desc.instance_count,
				desc.index_start,
				desc.base_vertex_index
			});

			u32 primitive_count;
			utils_get_primitive_type(desc.vertex_count, pipeline_info_->primitive_type, &primitive_count);
			primitive_count_ += primitive_count * desc.instance_count;
			++num_batches_;
		}
		void SetVertexBuffer(VertexBuffer* buffer) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetVertexBuffer);
			if(vertex_buffers_[0].get() == buffer)
				return;

			vertex_offsets_.fill(0);
			vertex_buffers_.fill(nullptr);
			vertex_buffers_[0] = ea::MakeShared(buffer);
			curr_vertex_buffer_hash_ = MakeHash(buffer);
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::vertex_buffer);
		}
		void SetVertexBuffers(const PODVector<VertexBuffer*> buffers, u32 instance_offset) override
		{
			SetVertexBuffers(buffers.Buffer(), buffers.Size(), instance_offset);
		}
		void SetVertexBuffers(const ea::vector<VertexBuffer*>& buffers, u32 instance_offset) override
		{
			SetVertexBuffers(const_cast<VertexBuffer**>(buffers.data()), buffers.size(), instance_offset);
		}
		void SetIndexBuffer(IndexBuffer* buffer) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetIndexBuffer);
			if(index_buffer_.get() == buffer)
				return;
			index_buffer_ = ea::MakeShared(buffer);
			if(buffer)
				index_type_ = buffer->GetIndexSize() == sizeof(u32) ? VT_UINT32 : VT_UINT16;

			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::index_buffer);
		}
		void SetShaders(const DrawCommandShadersDesc& desc) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetShaders);
			// TODO: add support for other shaders
			static ShaderVariation* s_shaders[MAX_SHADER_TYPES] = {};
			s_shaders[VS] = desc.vs;
			s_shaders[PS] = desc.ps;

			static ShaderVariation* s_pipeline_shaders[MAX_SHADER_TYPES] = {};
			s_pipeline_shaders[VS] = pipeline_info_->vs_shader;
			s_pipeline_shaders[PS] = pipeline_info_->ps_shader;

			if(enable_clip_planes_)
			{
				for(u8 i = 0; i < MAX_SHADER_TYPES; ++i)
				{
					if(!s_shaders[i])
						continue;
					s_shaders[i] = s_shaders[i]->GetOwner()->GetVariation(
						static_cast<ShaderType>(i),
						s_shaders[i]->GetDefinesClipPlane()
					);
				}
			}

			bool changed = false;
			for(u8 i = 0; i < MAX_SHADER_TYPES; ++i)
			{
				if(s_shaders[i] == s_pipeline_shaders[i])
					continue;

				if(i == VS)
					dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::vertex_decl) | static_cast<u32>(RenderCommandDirtyState::vertex_buffer);
				changed = true;
				break;
			}

			if (!changed)
				return;

			for(u8 i =0; i < MAX_SHADER_TYPES; ++i)
			{
				// TODO: schedule shader creation to run at worker thread
				if(s_shaders[i] && !s_shaders[i]->GetGPUObject())
				{
					if(!s_shaders[i]->GetCompilerOutput().Empty())
						s_shaders[i] = nullptr;
					else if(!s_shaders[i]->Create())
					{
						ATOMIC_LOGERROR("Failed to create shader: " + s_shaders[i]->GetName());
						s_shaders[i] = nullptr;
					}
				}

				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			}

			const auto vs_shader = s_shaders[VS];
			const auto ps_shader = s_shaders[PS];
			/*auto gs_shader = desc.gs;
			auto ds_shader = desc.ds;
			auto hs_shader = desc.hs;*/

			pipeline_info_->vs_shader = vs_shader;
			pipeline_info_->ps_shader = ps_shader;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);

			if(vs_shader && ps_shader)
			{
				const ShaderProgramQuery query{
					vs_shader,
					ps_shader
				};
				shader_program_ = GetOrCreateShaderProgram(query);

				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::shader_program);
			}
			else
			{
				shader_program_ = nullptr;
				dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::shader_program);
			}

			if (enable_clip_planes_)
				SetShaderParameter(VSP_CLIPPLANE, clip_plane_);
		}
		void SetShaderParameter(StringHash param, const float* data, u32 count) override
		{
			const FloatVector vec(data, count);
			const Variant var = vec;
			SetShaderParameter(param, var);
		}
		void SetShaderParameter(StringHash param, const FloatVector& value) override
		{
			const Variant var = value;
			SetShaderParameter(param, var);
		}
		void SetShaderParameter(StringHash param, float value) override
		{
			const Variant var = value;
			SetShaderParameter(param, var);
		}
		void SetShaderParameter(StringHash param, int value) override
		{
			const Variant var = value;
			SetShaderParameter(param, var);
		}
		void SetShaderParameter(StringHash param, bool value) override
		{
			const Variant var = value;
			SetShaderParameter(param, var);
		}
		void SetShaderParameter(StringHash param, const Color& value) override
		{
			const Variant var = value;
			SetShaderParameter(param, var);
		}
		void SetShaderParameter(StringHash param, const Vector2& value) override
		{
			const Variant var = value;
			SetShaderParameter(param, var);
		}
		void SetShaderParameter(StringHash param, const Vector3& value) override
		{
			const Variant var = value;
			SetShaderParameter(param, var);
		}
		void SetShaderParameter(StringHash param, const Vector4& value) override
		{
			const Variant var = value;
			SetShaderParameter(param, var);
		}
		void SetShaderParameter(StringHash param, const IntVector2& value) override
		{
			const Variant var = value;
			SetShaderParameter(param, var);
		}
		void SetShaderParameter(StringHash param, const IntVector3& value) override
		{
			const Variant var = value;
			SetShaderParameter(param, var);
		}
		void SetShaderParameter(StringHash param, const Matrix3& value) override
		{
			const Variant var = value;
			SetShaderParameter(param, var);
		}
		void SetShaderParameter(StringHash param, const Matrix3x4& value) override
		{
			const Variant var = value;
			SetShaderParameter(param, var);
		}
		void SetShaderParameter(StringHash param, const Matrix4& value) override
		{
			const Variant var = value;
			SetShaderParameter(param, var);
		}
		void SetShaderParameter(StringHash param, const Variant& value) override
		{
			params_2_update_.push({ param, value });
		}
		bool HasShaderParameter(StringHash param) override
		{
			if (!shader_program_)
				return false;
			ShaderParameter shader_param;
			return shader_program_->GetParameter(param, &shader_param);
		}
		bool NeedShaderGroupUpdate(ShaderParameterGroup group, const void* source) override
		{
			bool result = false;
			for(u8 i =0; i < MAX_SHADER_TYPES; ++i)
			{
				const auto idx = i * static_cast<u8>(MAX_SHADER_PARAMETER_GROUPS) + static_cast<u8>(group);
				const auto src = shader_param_sources_[idx];
				const auto target_src = reinterpret_cast<u32>(source);
				if(src == M_MAX_UNSIGNED || src != target_src)
				{
					result = true;
					shader_param_sources_[idx] = target_src;
				}
			}

			return result;
		}
		void SetTexture(TextureUnit unit, RenderSurface* surface) override
		{
			return SetTexture(unit, surface ? surface->GetParentTexture() : static_cast<Texture*>(nullptr));
		}
		void SetTexture(TextureUnit unit, Texture* texture) override
		{
			if (unit >= MAX_TEXTURE_UNITS)
				return;

			if(texture)
			{
				if(render_targets_[0] && render_targets_[0]->GetParentTexture() == texture)
					texture = texture->GetBackupTexture();
				else
				{
					// Resolve multisampled texture
					if(texture->GetMultiSample() > 1 && texture->GetAutoResolve() && texture->IsResolveDirty())
					{
						if (texture->GetType() == Texture2D::GetTypeStatic())
							ResolveTexture(static_cast<Texture2D*>(texture));
						if (texture->GetType() == TextureCube::GetTypeStatic())
							ResolveTexture(static_cast<TextureCube*>(texture));
					}
				}

				if (texture->GetLevelsDirty())
					texture->RegenerateLevels();
			}

			if(texture && texture->GetParametersDirty())
			{
				texture->UpdateParameters();
				textures_[unit] = {};
			}

			if (texture == textures_[unit].owner.get())
				return;

			const auto view = texture ? texture->GetShaderResourceView() : RefCntAutoPtr<ITextureView>();
			textures_[unit] = {
				nullptr,
				StringHash::ZERO,
				unit,
				view,
				ea::MakeShared(texture)
			};
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::textures);
		}
		void SetTexture(TextureUnit unit, RenderTexture* texture) override
		{
			SetTexture(unit, texture->GetBackBuffer().get());
		}
		bool HasTexture(TextureUnit unit) override
		{
			if (unit >= MAX_TEXTURE_UNITS || !shader_program_)
				return false;
			return shader_program_->GetSampler(unit) != nullptr;
		}
		void SetRenderTarget(u8 index, RenderSurface* surface) override
		{
			if (index >= MAX_RENDERTARGETS)
				return;

			if (render_targets_[index].get() == surface)
				return;

			render_targets_[index] = ea::MakeShared(surface);
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::render_targets);

			if (!surface)
				return;

			Texture* parent_texture = surface->GetParentTexture();
			for(u8 i =0; i < MAX_TEXTURE_UNITS; ++i)
			{
				if(textures_[i].owner.get() == parent_texture)
					SetTexture(static_cast<TextureUnit>(i), textures_[i].owner->GetBackupTexture());
			}

			if(parent_texture->GetMultiSample() > 1 && parent_texture->GetAutoResolve())
			{
				parent_texture->SetResolveDirty(true);
				surface->SetResolveDirty(true);
			}

			if (parent_texture->GetLevels() > 1)
				parent_texture->SetLevelsDirty();

			const auto format = surface->GetParentTexture()->GetFormat();
			if(pipeline_info_->output.render_target_formats[index] != format)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->output.render_target_formats[index] = format;
		}
		void SetRenderTarget(u8 index, Texture2D* texture) override
		{
			if (texture)
				SetRenderTarget(index, texture->GetRenderSurface());
			else
				SetRenderTarget(index, static_cast<RenderSurface*>(nullptr));
		}
		void SetRenderTarget(u8 index, RenderTexture* texture) override
		{
			SetDepthStencil(texture->GetDepthStencil().get());
			SetRenderTarget(index, texture->GetBackBuffer().get());
		}
		void SetDepthStencil(RenderSurface* surface) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetDepthStencil);
			if(depth_stencil_.get() == surface)
				return;

			depth_stencil_ = ea::MakeShared(surface);
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::render_targets);

			auto format = graphics_->GetImpl()->GetSwapChain()->GetDesc().DepthBufferFormat;
			if(surface)
				format = surface->GetParentTexture()->GetFormat();

			if(pipeline_info_->output.depth_stencil_format != format)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->output.depth_stencil_format = format;
		}
		void SetDepthStencil(Texture2D* texture) override
		{
			if (texture)
				SetDepthStencil(texture->GetRenderSurface());
			else
				SetDepthStencil(static_cast<RenderSurface*>(nullptr));
		}
		void ResetRenderTarget(u8 index) override
		{
			SetRenderTarget(index, static_cast<RenderSurface*>(nullptr));
		}
		void ResetRenderTargets() override
		{
			for (u8 i = 0; i < MAX_RENDERTARGETS; ++i)
				ResetRenderTarget(i);
		}
		void ResetDepthStencil() override
		{
			SetDepthStencil(static_cast<RenderSurface*>(nullptr));
		}
		void SetPrimitiveType(PrimitiveType type) override
		{
			if(pipeline_info_->primitive_type != type)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->primitive_type = type;
		}
		void SetViewport(const IntRect& viewport) override
		{
			const IntVector2 size = graphics_->GetRenderTargetDimensions();
			IntRect rect_cpy = viewport;

			if (rect_cpy.right_ <= rect_cpy.left_)
				rect_cpy.right_ = rect_cpy.left_ + 1;
			if (rect_cpy.bottom_ <= rect_cpy.top_)
				rect_cpy.bottom_ = rect_cpy.top_ + 1;
			rect_cpy.left_	 = Clamp(rect_cpy.left_, 0, size.x_);
			rect_cpy.top_	 = Clamp(rect_cpy.top_, 0, size.y_);
			rect_cpy.right_	 = Clamp(rect_cpy.right_, 0, size.x_);
			rect_cpy.bottom_ = Clamp(rect_cpy.bottom_, 0, size.y_);

			if(viewport_ == rect_cpy)
				return;

			viewport_ = rect_cpy;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::viewport);

			SetScissorTest(false);
		}
		void SetBlendMode(BlendMode mode, bool alpha_to_coverage) override
		{
			if(pipeline_info_->blend_mode != mode)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			if(pipeline_info_->alpha_to_coverage_enabled != alpha_to_coverage)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->blend_mode = mode;
			pipeline_info_->alpha_to_coverage_enabled = alpha_to_coverage;
		}
		void SetColorWrite(bool enable) override
		{
			if(pipeline_info_->color_write_enabled == enable)
				return;
			pipeline_info_->color_write_enabled = enable;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
		}
		void SetCullMode(CullMode mode) override
		{
			if(pipeline_info_->cull_mode == mode)
				return;
			pipeline_info_->cull_mode = mode;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
		}
		void SetDepthBias(float constant_bias, float slope_scaled_bias) override
		{
			if(pipeline_info_->constant_depth_bias == constant_bias && pipeline_info_->slope_scaled_depth_bias == slope_scaled_bias)
				return;
			pipeline_info_->constant_depth_bias = constant_bias;
			pipeline_info_->slope_scaled_depth_bias = slope_scaled_bias;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
		}
		void SetDepthTest(CompareMode mode) override
		{
			if(pipeline_info_->depth_cmp_function == mode)
				return;
			pipeline_info_->depth_cmp_function = mode;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
		}
		void SetDepthWrite(bool enable) override
		{
			if(pipeline_info_->depth_write_enabled == enable)
				return;
			pipeline_info_->depth_write_enabled = enable;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
		}
		void SetFillMode(FillMode mode) override
		{
			if(pipeline_info_->fill_mode == mode)
				return;
			pipeline_info_->fill_mode = mode;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
		}
		void SetLineAntiAlias(bool enable) override
		{
			if(pipeline_info_->stencil_test_enabled == enable)
				return;
			pipeline_info_->stencil_test_enabled = enable;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
		}
		void SetScissorTest(bool enable, const IntRect& rect) override
		{
			const IntVector2 rt_size = graphics_->GetRenderTargetDimensions();
			const IntVector2 view_pos(viewport_.left_, viewport_.top_);

			if(enable)
			{
				IntRect int_rect;
				int_rect.left_	 = Clamp(rect.left_	+ view_pos.x_, 0, rt_size.x_ - 1);
				int_rect.top_	 = Clamp(rect.top_		+ view_pos.y_, 0, rt_size.y_ - 1);
				int_rect.right_  = Clamp(rect.right_	+ view_pos.x_, 0, rt_size.x_);
				int_rect.bottom_ = Clamp(rect.bottom_	+ view_pos.y_, 0, rt_size.y_);

				if(int_rect.right_ == int_rect.left_)
					int_rect.right_++;
				if(int_rect.bottom_ == int_rect.top_)
					int_rect.bottom_++;

				if(int_rect.right_ < int_rect.left_ || int_rect.bottom_ < int_rect.top_)
					enable = false;

				if(enable && int_rect != scissor_)
				{
					scissor_ = int_rect;
					dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::scissor);
				}
			}

			if(pipeline_info_->scissor_test_enabled != enable)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->scissor_test_enabled = enable;
		}
		void SetScissorTest(bool enable, const Atomic::Rect& rect = Atomic::Rect::FULL, bool border_inclusive = true) override
		{
			// During some light rendering loops, a full rect is toggled on/off repeatedly.
			// Disable scissor in that case to reduce state changes
			if(rect.min_.x_ <= 0.0f && rect.min_.y_ <= 0.0f && rect.max_.x_ >= 1.0f && rect.max_.y_ >= 1.0f)
				enable = false;

			if(enable)
			{
				const IntVector2 rt_size = graphics_->GetRenderTargetDimensions();
				const IntVector2 view_size(viewport_.Size());
				const IntVector2 view_pos(viewport_.left_, viewport_.top_);
				const int expand = border_inclusive ? 1 : 0;
				IntRect int_rect;

				int_rect.left_	 = Clamp(static_cast<i32>((rect.min_.x_ + 1.0f) * 0.5f * view_size.x_) + view_pos.x_, 0, rt_size.x_ - 1);
				int_rect.top_	 = Clamp(static_cast<i32>((-rect.max_.y_ + 1.0f) * 0.5f * view_size.y_) + view_pos.y_, 0, rt_size.y_ - 1);
				int_rect.right_	 = Clamp(static_cast<i32>((rect.max_.x_ + 1.0f) * 0.5f * view_size.x_) + view_pos.x_ + expand, 0, rt_size.x_);
				int_rect.bottom_ = Clamp(static_cast<i32>((-rect.min_.y_ + 1.0f) * 0.5f * view_size.y_) + view_pos.y_ + expand, 0, rt_size.y_);

				if (int_rect.right_ == int_rect.left_)
					int_rect.right_++;
				if(int_rect.bottom_ == int_rect.top_)
					int_rect.bottom_++;

				if(int_rect.right_ < int_rect.left_ || int_rect.bottom_ < int_rect.top_)
					enable = false;

				if(enable && int_rect != scissor_)
				{
					scissor_ = int_rect;
					dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::scissor);
				}
			}

			if(pipeline_info_->scissor_test_enabled != enable)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);

			pipeline_info_->scissor_test_enabled = enable;
		}
		void SetStencilTest(const DrawCommandStencilTestDesc& desc) override
		{
			if(desc.enable != pipeline_info_->stencil_test_enabled)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->stencil_test_enabled = desc.enable;

			if (!desc.enable)
				return;

			if(desc.mode != pipeline_info_->stencil_cmp_function)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->stencil_cmp_function = desc.mode;

			if(desc.pass != pipeline_info_->stencil_op_on_passed)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->stencil_op_on_passed = desc.pass;

			if(desc.fail != pipeline_info_->stencil_op_on_stencil_failed)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->stencil_op_on_stencil_failed = desc.fail;

			if(desc.depth_fail != pipeline_info_->stencil_op_depth_failed)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->stencil_op_depth_failed = desc.depth_fail;

			if(desc.compare_mask != pipeline_info_->stencil_cmp_mask)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->stencil_cmp_mask = desc.compare_mask;

			if(desc.write_mask != pipeline_info_->stencil_write_mask)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->stencil_write_mask = desc.write_mask;

			stencil_ref_ = desc.stencil_ref;
		}
		void SetClipPlane(const DrawCommandClipPlaneDesc& desc) override
		{
			enable_clip_planes_ = desc.enable;
			if(!enable_clip_planes_)
				return;

			const Matrix4 view_proj = desc.projection * desc.view;
			clip_plane_ = desc.clip_plane.Transformed(view_proj).ToVector4();
			SetShaderParameter(VSP_CLIPPLANE, clip_plane_);
		}
		bool ResolveTexture(Texture2D* dest) override
		{
			throw std::exception("Not implemented");
		}
		bool ResolveTexture(Texture2D* dest, const IntRect& viewport) override
		{
			if(!dest || !dest->GetRenderSurface())
				return false;
			ATOMIC_PROFILE(IDrawCommand::ResolveTexture);

			const auto rt_size = graphics_->GetRenderTargetDimensions();
			IntRect vp_copy = viewport;
			if (vp_copy.right_ <= vp_copy.left_)
				vp_copy.right_ = vp_copy.left_ + 1;
			if (vp_copy.bottom_ <= vp_copy.top_)
				vp_copy.bottom_ = vp_copy.top_ + 1;

			Diligent::Box src_box;
			src_box.MinX = Clamp(vp_copy.left_, 0, rt_size.x_);
			src_box.MinY = Clamp(vp_copy.top_, 0, rt_size.y_);
			src_box.MaxX = Clamp(vp_copy.right_, 0, rt_size.x_);
			src_box.MaxY = Clamp(vp_copy.bottom_, 0, rt_size.y_);
			src_box.MinZ = 0;
			src_box.MaxZ = 1;

			// TODO: enable this when MSAA is available
			//bool resolve = multiSample_ > 1;
			bool resolve = false;
			auto source = graphics_->GetImpl()->GetSwapChain()->GetCurrentBackBufferRTV();
			auto destination_tex = dest->GetGPUObject().Cast<Diligent::ITexture>(Diligent::IID_Texture);

			if(!resolve)
			{
				Diligent::CopyTextureAttribs copy_attribs = {};
				copy_attribs.pSrcTexture = source->GetTexture();
				copy_attribs.pSrcBox = &src_box;
				copy_attribs.pDstTexture = destination_tex;
				context_->CopyTexture(copy_attribs);
			}
			else
			{
				Diligent::ResolveTextureSubresourceAttribs resolve_attribs = {};
				// if it is resolving to fullscreen, just resolve directly to the destination texture
				// otherwise we must need an auxiliary texture to resolve to
				if (!src_box.MinX && !src_box.MinY && src_box.MaxX == rt_size.x_ && src_box.MaxY == rt_size.y_)
				{
					context_->ResolveTextureSubresource(source->GetTexture(), destination_tex, resolve_attribs);
				}
				else
				{
					throw std::exception("Not implemented. You must implement MSAA first");
				}
			}

			return true;
		}
		bool ResolveTexture(TextureCube* dest) override
		{
			throw std::exception("Not implemented");
		}
		u32 GetPrimitiveCount() override { return primitive_count_; }
		u32 GetNumBatches() override { return num_batches_; }

		VertexBuffer* GetVertexBuffer(u8 index) override
		{
			return vertex_buffers_[index].get();
		}
		IndexBuffer* GetIndexBuffer() override
		{
			return index_buffer_.get();
		}
		ShaderVariation* GetShader(ShaderType type) override
		{
			static ShaderVariation* s_shaders[MAX_SHADER_TYPES] = {
				pipeline_info_->vs_shader,
				pipeline_info_->ps_shader,
			};
			if(type >= MAX_SHADER_TYPES)
				return nullptr;

			return s_shaders[type];
		}
		Texture* GetTexture(TextureUnit unit) override
		{
			if(unit >= MAX_TEXTURE_UNITS)
				return nullptr;
			return textures_[unit].owner.get();
		}
		RenderSurface* GetRenderTarget(u8 index) override
		{
			if(index >= MAX_RENDERTARGETS)
				return nullptr;
			return render_targets_[index].get();
		}
		RenderSurface* GetDepthStencil() override
		{
			return depth_stencil_.get();
		}
		PrimitiveType GetPrimitiveType() override
		{
			return pipeline_info_->primitive_type;
		}
		IntRect GetViewport() override
		{
			return viewport_;
		}
		BlendMode GetBlendMode() override
		{
			return pipeline_info_->blend_mode;
		}
		bool GetAlphaToCoverage() override
		{
			return pipeline_info_->alpha_to_coverage_enabled;
		}
		bool GetColorWrite() override
		{
			return pipeline_info_->color_write_enabled;
		}
		CullMode GetCullMode() override
		{
			return pipeline_info_->cull_mode;
		}
		float GetDepthBias() override
		{
			return pipeline_info_->constant_depth_bias;
		}
		float GetSlopeScaledDepthBias() override
		{
			return pipeline_info_->slope_scaled_depth_bias;
		}
		CompareMode GetDepthTest() override
		{
			return pipeline_info_->depth_cmp_function;
		}
		bool GetDepthWrite() override
		{
			return pipeline_info_->depth_write_enabled;
		}
		FillMode GetFillMode() override
		{
			return pipeline_info_->fill_mode;
		}
		bool GetLineAntiAlias() override
		{
			return pipeline_info_->stencil_test_enabled;
		}
		bool GetScissorTest() override
		{
			return pipeline_info_->scissor_test_enabled;
		}
		IntRect GetScissorRect() override
		{
			return scissor_;
		}
		bool GetStencilTest() override
		{
			return pipeline_info_->stencil_test_enabled;
		}
		CompareMode GetStencilTestMode() override
		{
			return pipeline_info_->stencil_cmp_function;
		}
		StencilOp GetStencilPass() override
		{
			return pipeline_info_->stencil_op_on_passed;
		}
		StencilOp GetStencilFail() override
		{
			return pipeline_info_->stencil_op_on_stencil_failed;
		}
		StencilOp GetStencilZFail() override
		{
			return pipeline_info_->stencil_op_depth_failed;
		}
		u32 GetStencilRef() override
		{
			return stencil_ref_;
		}
		u32 GetStencilCompareMask() override
		{
			return pipeline_info_->stencil_cmp_mask;
		}
		u32 GetStencilWriteMask() override
		{
			return pipeline_info_->stencil_write_mask;
		}
		bool GetClipPlane() override
		{
			return enable_clip_planes_;
		}
		ShaderProgram* GetShaderProgram() override
		{
			return shader_program_;
		}
		IntVector2 GetRenderTargetDimensions() override
		{
			int width, height;
			if(render_targets_[0])
			{
				width = render_targets_[0]->GetWidth();
				height = render_targets_[0]->GetHeight();
			}
			else if(depth_stencil_)
			{
				width = depth_stencil_->GetWidth();
				height = depth_stencil_->GetHeight();
			}
			else
			{
				width = graphics_->GetWidth();
				height = graphics_->GetHeight();
			}

			return IntVector2(width, height);
		}
	private:
		bool PrepareRenderTargets()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareRenderTargets);
			if ((dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::render_targets)) == 0)
				return false;
			// Remove dirty state
			dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::render_targets);

			// render target must be set in order.
			s_num_rts = 0;

			for(u8 i =0; i < MAX_RENDERTARGETS; ++i)
			{
				// Skip if slot is empty.
				if (!render_targets_[i])
					continue;
				const auto rt = render_targets_[i]->GetRenderTargetView();
				s_render_targets[s_num_rts]= rt;

				if(pipeline_info_->output.render_target_formats[s_num_rts] != rt->GetTexture()->GetDesc().Format)
				{
					pipeline_info_->output.render_target_formats[s_num_rts] = rt->GetTexture()->GetDesc().Format;
					dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
				}

				++s_num_rts;
			}

			const auto wnd_size = graphics_->GetSize();
			const auto depth_stencil = depth_stencil_;
			const auto depth_stencil_size = IntVector2(
				depth_stencil ? depth_stencil->GetWidth() : 0, 
				depth_stencil ? depth_stencil->GetHeight() : 0
			);

			if(pipeline_info_->output.num_rts != s_num_rts)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);

			pipeline_info_->output.num_rts = s_num_rts;
			if (s_num_rts > 0) 
				return true;

			if (depth_stencil && depth_stencil_size != wnd_size)
				return true;

			s_num_rts = 1;
			s_render_targets[0] = graphics_->GetImpl()->GetSwapChain()->GetCurrentBackBufferRTV();

			if(s_num_rts != pipeline_info_->output.num_rts)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->output.num_rts = s_num_rts;

			if (pipeline_info_->output.render_target_formats[0] == s_render_targets[0]->GetTexture()->GetDesc().Format)
				return true;

			pipeline_info_->output.render_target_formats[0] = s_render_targets[0]->GetTexture()->GetDesc().Format;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);

			return true;
		}
		bool PrepareDepthStencil()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareDepthStencil);
			if (dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::render_targets) == 0)
				return false;

			auto depth_stencil = depth_stencil_ && depth_stencil_->GetUsage() == TEXTURE_DEPTHSTENCIL ?
				depth_stencil_->GetRenderTargetView() :
				graphics_->GetImpl()->GetSwapChain()->GetDepthBufferDSV();
			const auto wnd_size = graphics_->GetRenderTargetDimensions();

			if(!pipeline_info_->depth_write_enabled && depth_stencil_ && depth_stencil_->GetReadOnlyView())
				depth_stencil = depth_stencil_->GetReadOnlyView();
			else if(render_targets_[0] && !depth_stencil_)
			{
				const auto rt_size = IntVector2(
					render_targets_[0]->GetWidth(),
					render_targets_[0]->GetHeight()
				);
				if (rt_size.x_ < wnd_size.x_ || rt_size.y_ < wnd_size.y_)
					depth_stencil = nullptr;
			}

			s_depth_stencil = depth_stencil;
			return true;
		}
		void PreparePipelineState()
		{
			ATOMIC_PROFILE(IDrawCommand::PreparePipelineState);
			if ((dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::pipeline)) == 0)
				return;

			auto hash = pipeline_info_->ToHash();
			if(hash == curr_pipeline_hash_)
				return;
			pipeline_state_ = pipeline_state_builder_acquire(graphics_->GetImpl(), *pipeline_info_, hash);
			curr_pipeline_hash_ = hash;
			dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::pipeline);
			// If pipeline state is changed, we need to update shader resource binding.
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::srb)
				| static_cast<u32>(RenderCommandDirtyState::commit_pipeline);
		}
		void PrepareSRB()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareSRB);
			if(!shader_resource_binding_)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::srb);

			if ((dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::srb)) == 0)
				return;

			ShaderResourceBindingCreateDesc create_desc;
			create_desc.pipeline_hash = curr_pipeline_hash_;
			create_desc.resources = &textures_;
			create_desc.driver = graphics_->GetImpl();
			shader_resource_binding_ = pipeline_state_builder_get_or_create_srb(create_desc);
			dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::srb);
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::commit_srb);
		}
		void PrepareVertexBuffers()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareVertexBuffers);
			// Other Draw Commands can change vertex buffer, so we need to check if it is changed.
			if(s_vertex_buffer_hash != curr_vertex_buffer_hash_)
			{
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::vertex_buffer);
				s_vertex_buffer_hash = curr_vertex_buffer_hash_;
			}

			if((dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::vertex_buffer)) == 0)
				return;

			dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::vertex_buffer);

			u32 next_idx = 0;
			u32 new_vertex_decl_hash = 0;
			for(const auto& buffer : vertex_buffers_)
			{
				if(!buffer)
					continue;
				CombineHash(new_vertex_decl_hash, buffer->GetBufferHash(next_idx));
				s_vertex_buffers[next_idx] = buffer->GetGPUObject().Cast<Diligent::IBuffer>(Diligent::IID_Buffer);
				++next_idx;
			}

			if (next_idx == 0)
				return;

			context_->SetVertexBuffers(
				0,
				next_idx,
				s_vertex_buffers.data(),
				vertex_offsets_.data(),
				RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
				SET_VERTEX_BUFFERS_FLAG_NONE);

			if (!new_vertex_decl_hash)
				return;

			if(pipeline_info_->vs_shader)
			{
				CombineHash(new_vertex_decl_hash, pipeline_info_->vs_shader->ToHash());
				CombineHash(new_vertex_decl_hash, pipeline_info_->vs_shader->GetElementHash());
			}

			if(curr_vertx_decl_hash_ != new_vertex_decl_hash)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::vertex_decl);
			curr_vertx_decl_hash_ = new_vertex_decl_hash;
		}
		void PrepareIndexBuffer()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareIndexBuffer);
			if (!index_buffer_)
				return;
			if ((dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::index_buffer)) == 0)
				return;

			dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::index_buffer);

			const auto buffer = index_buffer_->GetGPUObject().Cast<Diligent::IBuffer>(Diligent::IID_Buffer);

			if(buffer != s_index_buffer)
				context_->SetIndexBuffer(buffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			s_index_buffer = buffer;
		}
		void PrepareVertexDeclarations()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareVertexDeclarations);
			if((dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::vertex_decl)) == 0)
				return;
			dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::vertex_decl);

			const auto vertex_decl = graphics_state_get_vertex_declaration(curr_vertx_decl_hash_);
			if(vertex_decl)
			{
				if(vertex_decl != vertex_declaration_)
				{
					dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
					pipeline_info_->input_layout = vertex_decl->GetInputLayoutDesc();
				}
				vertex_declaration_ = vertex_decl;
				return;
			}

			VertexDeclarationCreationDesc creation_desc;
			creation_desc.graphics = graphics_;
			creation_desc.hash = curr_vertx_decl_hash_;
			creation_desc.vertex_buffers = &vertex_buffers_;
			creation_desc.vertex_shader = pipeline_info_->vs_shader;
			vertex_declaration_ = new VertexDeclaration(creation_desc);

			if(!vertex_declaration_->GetNumInputs())
			{
				vertex_declaration_ = nullptr;
				return;
			}

			graphics_state_set_vertex_declaration(curr_vertx_decl_hash_, vertex_declaration_);

			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->input_layout = vertex_declaration_->GetInputLayoutDesc();
		}
		void PrepareTextures()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareTextures);

			if (!shader_program_)
				return;
			if((dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::textures)) == 0)
				return;
			dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::textures);

			u32 next_sampler_idx = 0;
			for(auto& desc : textures_)
			{
				if(!desc.owner)
					continue;

				const auto sampler = shader_program_->GetSampler(desc.unit);
				if (!sampler)
					continue;
				if(desc.name_hash != sampler->hash)
					dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::srb);

				desc.name = sampler->name.CString();
				desc.name_hash = sampler->hash;
				// get some hashes
				const u32 sampler_hash = pipeline_info_->immutable_samplers[next_sampler_idx].sampler.ToHash();
				const auto name_hash = pipeline_info_->immutable_samplers[next_sampler_idx].name_hash;
				// fill immutable sampler sampler
				desc.owner->GetSamplerDesc(pipeline_info_->immutable_samplers[next_sampler_idx].sampler);
				// if sampler or immutable sampler name has been changed. we must rebuild pipeline.
				if(sampler_hash != pipeline_info_->immutable_samplers[next_sampler_idx].sampler.ToHash() 
					|| sampler->hash != name_hash)
					dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);

				// put the name and hash name inside immutable sampler
				pipeline_info_->immutable_samplers[next_sampler_idx].name_hash = sampler->hash;
				pipeline_info_->immutable_samplers[next_sampler_idx].name = desc.name;
				++next_sampler_idx;
			}

			pipeline_info_->num_samplers = next_sampler_idx;
		}
		void PrepareParametersToUpload()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareParametersToUpload);
			if (params_2_update_.empty() || !shader_program_)
				return;

			for(;!params_2_update_.empty(); params_2_update_.pop())
			{
				auto& desc = params_2_update_.front();
				ShaderParameter parameter;
				if(!shader_program_->GetParameter(desc.name, &parameter))
				{
					s_empty_params_queue.push(desc);
					continue;
				}

				const auto buffer = static_cast<ConstantBuffer*>(parameter.bufferPtr_);
				if(!buffer)
					continue;

				render_command_write_param(buffer, parameter.offset_, &desc.value);
			}

			ea::swap(s_empty_params_queue, params_2_update_);
			graphics_->GetImpl()->UploadBufferChanges();
		}

		void PrepareClear()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareClear);
			auto changed = PrepareDepthStencil();
			changed = PrepareRenderTargets() || changed;
			if (changed)
				BoundRenderTargets();
		}
		void PrepareDraw()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareDraw);
			auto changed_rts = PrepareDepthStencil();
			changed_rts = PrepareRenderTargets() || changed_rts;

			PrepareVertexBuffers();
			PrepareIndexBuffer();
			PrepareVertexDeclarations();
			PrepareTextures();
			PreparePipelineState();
			PrepareSRB();
			PrepareParametersToUpload();

			if(changed_rts)
				BoundRenderTargets();

			const auto rt_size = graphics_->GetRenderTargetDimensions();
			if(dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::viewport))
			{
				dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::viewport);

				const auto rect = viewport_;
                Diligent::Viewport viewport;
                viewport.TopLeftX = static_cast<float>(rect.left_);
                viewport.TopLeftY = static_cast<float>(rect.top_);
                viewport.Width = static_cast<float>(rect.right_ - rect.left_);
                viewport.Height = static_cast<float>(rect.bottom_ - rect.top_);
                viewport.MinDepth = 0.0f;
                viewport.MaxDepth = 1.0f;
				context_->SetViewports(1, &viewport, rt_size.x_, rt_size.y_);
			}

			if(dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::scissor) && pipeline_info_->scissor_test_enabled)
			{
				dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::scissor);

				const auto rect = scissor_;
				const Diligent::Rect scissor_rect = {
									rect.left_,
									rect.top_,
									rect.right_,
									rect.bottom_
								};
				context_->SetScissorRects(1, &scissor_rect, rt_size.x_, rt_size.y_);
			}

			context_->SetStencilRef(stencil_ref_);

			static constexpr float s_blend_factors[] = { .0f, .0f, .0f, .0f };
			context_->SetBlendFactors(s_blend_factors);

			if(pipeline_state_ && dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::commit_pipeline))
			{
				dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::commit_pipeline);
				context_->SetPipelineState(pipeline_state_);
			}
			if(shader_resource_binding_ && dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::commit_srb))
			{
				dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::commit_srb);
				context_->CommitShaderResources(shader_resource_binding_, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			}
		}
		void BoundRenderTargets() const
		{
			context_->SetRenderTargets(s_num_rts, s_render_targets.data(), s_depth_stencil, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		}
		void ClearByHardware(const DrawCommandClearDesc& desc) const
		{
			ATOMIC_PROFILE(IDrawCommand::ClearByHardware);
				
			auto clear_stencil_flags = Diligent::CLEAR_DEPTH_FLAG_NONE;
			if(desc.flags & CLEAR_COLOR && s_render_targets[0])
				context_->ClearRenderTarget(s_render_targets[0], desc.color.Data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			if((desc.flags & (CLEAR_DEPTH | CLEAR_STENCIL)) !=0 && s_depth_stencil)
			{
				if(desc.flags & CLEAR_DEPTH)
					clear_stencil_flags |= Diligent::CLEAR_DEPTH_FLAG;
				if(desc.flags & CLEAR_STENCIL)
					clear_stencil_flags |= Diligent::CLEAR_STENCIL_FLAG;

				context_->ClearDepthStencil(s_depth_stencil, 
					clear_stencil_flags, 
					desc.depth, 
					static_cast<u8>(desc.stencil), 
					Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			}
		}
		void ClearBySoftware(const DrawCommandClearDesc& desc)
		{
			ATOMIC_PROFILE(IDrawCommand::ClearBySoftware);
			const auto renderer = graphics_->GetContext()->GetSubsystem<Renderer>();
			if (!renderer)
			{
				ATOMIC_LOGWARNING("Can´t clear without Renderer. Skipping!");
				return;
			}

			// Clear assigns an own vertex and index buffer, in this case we must
			// reset internal state.
			s_vertex_buffer_hash = 0;
			s_index_buffer = nullptr;

			// Clear command by software requires to do a lot of things
			// First we need to get the quad geometry from the renderer. (TODO: this will be used shader procedural triangle)
			// Second we need to get clear framebuffer shaders
			// Third we need to get or create a shader program to get access to constant buffer parameter positions
			// Fourth we declare the basic setup of our clear pipeline state and create pipeline state and SRB.
			// Fifth we write our data into constant buffers
			// Sixth, and finally we set up our device context and send draw command to GPU.
			const auto geometry = renderer->GetQuadGeometry();
			auto model_transform = Matrix4::IDENTITY;
			auto proj = Matrix4::IDENTITY;
			auto clear_color = desc.color;
			const auto vs_shader = graphics_->GetShader(VS, "ClearFramebuffer");
			const auto ps_shader = graphics_->GetShader(PS, "ClearFramebuffer");
			const auto program = GetOrCreateShaderProgram({ vs_shader, ps_shader });

			PipelineStateInfo pipeline_info = {};
			pipeline_info.debug_name = "Clear";
			pipeline_info.vs_shader = vs_shader;
			pipeline_info.ps_shader = ps_shader;
			pipeline_info.output.render_target_formats[0] = render_targets_[0]->GetParentTexture()->GetFormat();
			pipeline_info.output.num_rts = 1;
			pipeline_info.blend_mode = BLEND_REPLACE;
			pipeline_info.alpha_to_coverage_enabled = false;
			pipeline_info.cull_mode = CULL_NONE;
			pipeline_info.depth_cmp_function = CMP_ALWAYS;
			pipeline_info.depth_write_enabled = desc.flags & CLEAR_DEPTH;
			pipeline_info.fill_mode = FILL_SOLID;
			pipeline_info.scissor_test_enabled = false;
			pipeline_info.stencil_test_enabled = desc.flags & CLEAR_STENCIL;
			pipeline_info.stencil_cmp_function = CMP_ALWAYS;
			pipeline_info.stencil_op_on_passed = OP_REF;
			pipeline_info.stencil_op_on_stencil_failed = OP_KEEP;
			pipeline_info.stencil_op_depth_failed = OP_KEEP;
			pipeline_info.primitive_type = TRIANGLE_LIST;

			ShaderResourceTextures textures_dummy = {};
			u32 pipeline_hash = 0;

			auto pipeline = pipeline_state_builder_acquire(graphics_->GetImpl(), pipeline_info, pipeline_hash);
			auto srb = pipeline_state_builder_get_or_create_srb({
				graphics_->GetImpl(),
				pipeline_hash,
				&textures_dummy
				});

			WriteShaderParameter(program, VSP_MODEL, &model_transform, sizeof(Matrix4));
			WriteShaderParameter(program, VSP_VIEWPROJ, &proj, sizeof(Matrix4));
			WriteShaderParameter(program, PSP_MATDIFFCOLOR, &clear_color, sizeof(Matrix4));

			DrawIndexedAttribs draw_attribs = {};
			draw_attribs.IndexType = GetIndexSize(geometry->GetIndexBuffer()->GetIndexSize());
			draw_attribs.BaseVertex = geometry->GetVertexStart();
			draw_attribs.FirstIndexLocation = geometry->GetIndexStart();
			draw_attribs.NumInstances = 1;
			draw_attribs.NumIndices = geometry->GetIndexCount();
			draw_attribs.Flags = DRAW_FLAG_NONE;

			// TODO: use procedural shader triangle instead of vertex and index buffer
			context_->SetVertexBuffers(
				0,
				1,
				s_vertex_buffers.data(),
				nullptr,
				RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
				SET_VERTEX_BUFFERS_FLAG_RESET);
			context_->SetIndexBuffer(nullptr, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			context_->SetStencilRef(desc.stencil);
			context_->SetPipelineState(pipeline);
			context_->CommitShaderResources(srb, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			context_->DrawIndexed(draw_attribs);
		}

		void SetVertexBuffers(VertexBuffer** buffers, u32 count, u32 instance_offset)
		{
			if (count > MAX_VERTEX_STREAMS)
				ATOMIC_LOGWARNING("Too many vertex buffers");

			curr_vertex_buffer_hash_ = 0;
			count = Min(count, MAX_VERTEX_STREAMS);
			for(u32 i =0; i < MAX_VERTEX_STREAMS; ++i)
			{
				const auto buffer = i < count ? buffers[i] : nullptr;

				if(buffer)
				{
					CombineHash(curr_vertex_buffer_hash_, MakeHash(buffer));
					CombineHash(curr_vertex_buffer_hash_, buffer->GetBufferHash(i));
					const auto& elements = buffer->GetElements();
					// Check if buffer has per-instance data
					const auto has_instance_data = elements.Size() && elements[0].perInstance_;
					const auto offset = has_instance_data ? instance_offset * buffer->GetVertexSize() : 0;

					if(buffer != vertex_buffers_[i].get() || offset != vertex_offsets_[i])
						dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::vertex_buffer);
					vertex_buffers_[i] = ea::MakeShared(buffer);
					vertex_offsets_[i] = offset;
					continue;
				}

				if(vertex_buffers_[i])
				{
					vertex_buffers_[i] = nullptr;
					vertex_offsets_[i] = 0;
					dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::vertex_buffer);
				}
			}
		}

		SharedPtr<ShaderProgram> GetOrCreateShaderProgram(const ShaderProgramQuery& query) const
		{
			auto result = graphics_state_get_shader_program(query);
			if (result)
				return result;

			ShaderProgramCreationDesc creation_desc;
			creation_desc.graphics = graphics_;
			creation_desc.vertex_shader = query.vertex_shader;
			creation_desc.pixel_shader = query.pixel_shader;
			result = new ShaderProgram(creation_desc);
			graphics_state_set_shader_program(result);

			return result;
		}

		static void WriteShaderParameter(ShaderProgram* program, const StringHash& param, void* data, u32 length)
		{
			ShaderParameter shader_param;
			if (!program->GetParameter(param, &shader_param))
				return;
			if (!shader_param.bufferPtr_)
				return;
			static_cast<ConstantBuffer*>(shader_param.bufferPtr_)->SetParameter(shader_param.offset_, length, data);
		}
		static Diligent::VALUE_TYPE GetIndexSize(u32 size)
		{
			return size == sizeof(u16) ? Diligent::VT_UINT16 : Diligent::VT_UINT32;
		}

		Graphics* graphics_;
		Diligent::IDeviceContext* context_;
		PipelineStateInfo* pipeline_info_;
		Diligent::RefCntAutoPtr<Diligent::IPipelineState> pipeline_state_;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> shader_resource_binding_;

		ea::shared_ptr<RenderSurface> depth_stencil_;
		ea::queue<ShaderParameterUpdateDesc> params_2_update_;

		// arrays
		ea::array<ea::shared_ptr<RenderSurface>, MAX_RENDERTARGETS> render_targets_;
		ea::array<ShaderResourceTextureDesc, MAX_TEXTURE_UNITS> textures_;
		ea::array<ea::shared_ptr<VertexBuffer>, MAX_VERTEX_STREAMS> vertex_buffers_;
		ea::array<u64, MAX_VERTEX_STREAMS> vertex_offsets_;
		ea::array<u32, MAX_SHADER_PARAMETER_GROUPS * MAX_SHADER_TYPES> shader_param_sources_;

		ea::shared_ptr<IndexBuffer> index_buffer_;

		SharedPtr<VertexDeclaration> vertex_declaration_;
		SharedPtr<ShaderProgram> shader_program_;

		bool enable_clip_planes_{};
		Vector4 clip_plane_{};

		IntRect scissor_{};
		IntRect viewport_{};

		ValueType index_type_{};

		u8 stencil_ref_{};

		u32 dirty_flags_{};

		u32 curr_vertx_decl_hash_{};
		u32 curr_vertex_buffer_hash_{};
		u32 curr_pipeline_hash_{};

		u32 primitive_count_;
		u32 num_batches_;
	};

	Atomic::IDrawCommand* graphics_create_command(Atomic::Graphics* graphics)
	{
		Atomic::IDrawCommand* result = new DrawCommandImpl(graphics, graphics->GetImpl()->GetDeviceContext());
		return result;
	}
}
