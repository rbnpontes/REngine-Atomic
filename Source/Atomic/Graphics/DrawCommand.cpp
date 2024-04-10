#include "DrawCommand.h"

#include "Geometry.h"
#include "Model.h"
#include "Renderer.h"
#include "Shader.h"
#include "../RHI/RenderCommand.h"
#include "../RHI/PipelineStateBuilder.h"
#include "../Core/Profiler.h"
#include "../IO/Log.h"
#include "../RHI/GraphicsState.h"

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
			curr_vertx_decl_hash(0),
			curr_vertex_buffer_hash(0)
		{
			pipeline_info_ = new PipelineStateInfo();
		}
		~DrawCommandImpl()
		{
			// TODO: clear static states too
			delete pipeline_info_;
		}

		void Reset() override
		{
			*pipeline_info_ = PipelineStateInfo{};
			pipeline_info_->output.render_target_formats.fill(Diligent::TEX_FORMAT_UNKNOWN);

			const auto wnd_size = graphics_->GetRenderTargetDimensions();
			viewport_ = IntRect(0, 0, wnd_size.x_, wnd_size.y_);

			pipeline_state_ = nullptr;
			vertex_declaration_ = nullptr;
			shader_program_ = nullptr;
			depth_stencil_ = nullptr;
			index_buffer_ = nullptr;
			shader_resource_binding_ = nullptr;

			params_2_update_ = {};

			render_targets_.fill(nullptr);
			textures_.fill(ShaderResourceTextureDesc{});
			vertex_buffers_.fill(nullptr);
			vertex_offsets_.fill(0);

			enable_clip_planes_ = false;
			clip_plane_ = Vector4::ZERO;
			curr_pipeline_hash_ = curr_vertex_buffer_hash = curr_vertx_decl_hash = 0u;

			dirty_flags_ = static_cast<u32>(RenderCommandDirtyState::all);
			num_batches_ = 0;
			primitive_count_ = 0;
		}
		void Clear(const DrawCommandClearDesc& desc) override
		{
			ATOMIC_PROFILE(IDrawCommand::Clear);
			const auto rt_size = graphics_->GetRenderTargetDimensions();

			PrepareClear();

			if(!viewport_.left_ && !viewport_.top_ && viewport_.right_ == rt_size.x_ && viewport_.bottom_ == rt_size.y_)
			{
				ClearByHardware(desc);
				return;
			}

			ClearBySoftware(desc);
		}
		void Draw(const DrawCommandDrawDesc& desc) override
		{
			ATOMIC_PROFILE(IDrawCommand::Draw);

			PrepareDraw();

			context_->DrawIndexed({
				desc.index_count,
				desc.index_type,
				DRAW_FLAG_NONE,
				1,
				desc.index_start,
				desc.base_vertex_index,
				1
			});
		}
		void Draw(const DrawCommandInstancedDrawDesc& desc) override
		{
			ATOMIC_PROFILE(IDrawCommand::Draw);

			PrepareDraw();

			context_->DrawIndexed({
				desc.index_count,
				desc.index_type,
				DRAW_FLAG_NONE,
				desc.instance_count,
				desc.index_start,
				desc.base_vertex_index,
				1
			});
		}
		void SetVertexBuffer(VertexBuffer* buffer) override
		{
			if(vertex_buffers_[0] != buffer)
				return;

			vertex_offsets_.fill(0);
			vertex_buffers_.fill(nullptr);
			vertex_buffers_[0] = buffer;
			curr_vertex_buffer_hash = MakeHash(buffer);
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
			if(index_buffer_ == buffer)
				return;
			index_buffer_ = buffer;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::index_buffer);
		}
		void SetShaders(const DrawCommandShadersDesc& desc) override
		{

			// TODO: add support for other shaders
			static ShaderVariation* s_shaders[MAX_SHADER_TYPES] = {
				desc.vs,
				desc.ps,
			};
			static ShaderVariation* s_pipeline_shaders[MAX_SHADER_TYPES] = {
				pipeline_info_->vs_shader,
				pipeline_info_->ps_shader,
			};

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

			auto vs_shader = s_shaders[VS];
			auto ps_shader = s_shaders[PS];
			/*auto gs_shader = desc.gs;
			auto ds_shader = desc.ds;
			auto hs_shader = desc.hs;*/

			if(vs_shader && ps_shader)
			{
				ShaderProgramQuery query{
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

		u32 GetPrimitiveCount() override { return primitive_count_; }
		u32 GetNumBatches() override { return num_batches_; }
	private:
		bool PrepareRenderTargets()
		{
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

			const auto wnd_size = graphics_->GetRenderTargetDimensions();
			const auto depth_stencil_size = IntVector2(
				depth_stencil_ ? depth_stencil_->GetWidth() : 0, 
				depth_stencil_ ? depth_stencil_->GetHeight() : 0
			);

			if(pipeline_info_->output.num_rts != s_num_rts)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);

			pipeline_info_->output.num_rts = s_num_rts;
			if (s_num_rts > 0) 
				return true;

			if (depth_stencil_ || depth_stencil_size != wnd_size)
				return true;

			s_num_rts = 1;
			s_render_targets[0] = graphics_->GetImpl()->GetSwapChain()->GetCurrentBackBufferRTV();

			if(s_num_rts != pipeline_info_->output.num_rts)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);

			if (pipeline_info_->output.render_target_formats[0] == s_render_targets[0]->GetTexture()->GetDesc().Format)
				return true;

			pipeline_info_->output.render_target_formats[0] = s_render_targets[0]->GetTexture()->GetDesc().Format;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);

			return true;
		}
		bool PrepareDepthStencil()
		{
			if (dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::depth_stencil))
				return false;
			dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::depth_stencil);

			s_depth_stencil = depth_stencil_ ? 
				depth_stencil_->GetRenderTargetView() :
				graphics_->GetImpl()->GetSwapChain()->GetDepthBufferDSV();
			const auto wnd_size = graphics_->GetRenderTargetDimensions();

			if(!pipeline_info_->depth_write_enabled && depth_stencil_ && depth_stencil_->GetReadOnlyView())
				s_depth_stencil = depth_stencil_->GetReadOnlyView();
			else if(render_targets_[0] && !depth_stencil_)
			{
				const auto rt_size = IntVector2(
					render_targets_[0]->GetWidth(),
					render_targets_[0]->GetHeight()
				);
				if (rt_size.x_ < wnd_size.x_ || rt_size.y_ < wnd_size.y_)
					s_depth_stencil = nullptr;
			}

			return true;
		}
		void PreparePipelineState()
		{
			if ((dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::pipeline)) == 0)
				return;

			auto hash = pipeline_info_->ToHash();
			if(hash == curr_pipeline_hash_)
				return;
			pipeline_state_ = pipeline_state_builder_acquire(graphics_->GetImpl(), *pipeline_info_, hash);
			curr_pipeline_hash_ = hash;
			dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::pipeline);
			// If pipeline state is changed, we need to update shader resource binding.
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::srb);
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
		}
		void PrepareVertexBuffers()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareVertexBuffers);
			// Other Draw Commands can change vertex buffer, so we need to check if it is changed.
			if(s_vertex_buffer_hash != curr_vertex_buffer_hash)
			{
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::vertex_buffer);
				s_vertex_buffer_hash = curr_vertex_buffer_hash;
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

			curr_vertx_decl_hash = new_vertex_decl_hash;
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
			if((dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::vertex_decl)) == 0)
				return;
			dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::vertex_decl);

			const auto vertex_decl = graphics_state_get_vertex_declaration(curr_vertx_decl_hash);
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
			creation_desc.hash = curr_vertx_decl_hash;
			creation_desc.vertex_buffers = &vertex_buffers_;
			creation_desc.vertex_shader = pipeline_info_->vs_shader;
			vertex_declaration_ = new VertexDeclaration(creation_desc);
			graphics_state_set_vertex_declaration(curr_vertx_decl_hash, vertex_declaration_);

			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->input_layout = vertex_decl->GetInputLayoutDesc();
		}
		void PrepareParametersToUpload()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareParametersToUpload);
			if (params_2_update_.empty() || !shader_program_)
				return;

			for(;!params_2_update_.empty(); params_2_update_.pop())
			{
				const auto& desc = params_2_update_.front();
				ShaderParameter parameter;
				if(!shader_program_->GetParameter(desc.name, &parameter))
				{
					s_empty_params_queue.push(desc);
					continue;
				}

				const auto buffer = static_cast<ConstantBuffer*>(parameter.bufferPtr_);
				if(!buffer)
					continue;
				render_command_write_param(buffer, parameter.offset_, desc.value);
			}

			ea::swap(s_empty_params_queue, params_2_update_);
			graphics_->GetImpl()->UploadBufferChanges();
		}

		void PrepareClear()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareClear);
			const auto changed = PrepareDepthStencil() || PrepareDepthStencil();
			if (changed)
				BoundRenderTargets();
		}
		void PrepareDraw()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareDraw);
			const auto changed_rts = PrepareDepthStencil() || PrepareRenderTargets();

			PrepareVertexBuffers();
			PrepareIndexBuffer();
			PrepareVertexDeclarations();

			PreparePipelineState();
			PrepareSRB();
			PrepareParametersToUpload();

			if(changed_rts)
				BoundRenderTargets();

			if(pipeline_state_)
				context_->SetPipelineState(pipeline_state_);
			if(shader_resource_binding_)
				context_->CommitShaderResources(shader_resource_binding_, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		}
		void BoundRenderTargets() const
		{
			context_->SetRenderTargets(s_num_rts, s_render_targets.data(), s_depth_stencil, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		}
		void ClearByHardware(const DrawCommandClearDesc& desc)
		{
			ATOMIC_PROFILE(IDrawCommand::ClearByHardware);
				
			auto clear_stencil_flags = Diligent::CLEAR_DEPTH_FLAG_NONE;
			if(desc.flags & CLEAR_COLOR && render_targets_[0])
				context_->ClearRenderTarget(s_render_targets[0], desc.color.Data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			if((desc.flags & (CLEAR_DEPTH | CLEAR_STENCIL)) !=0 && depth_stencil_)
			{
				if(desc.flags & CLEAR_DEPTH)
					clear_stencil_flags |= Diligent::CLEAR_DEPTH_FLAG;
				if(desc.flags & CLEAR_STENCIL)
					clear_stencil_flags |= Diligent::CLEAR_STENCIL_FLAG;

				context_->ClearDepthStencil(s_depth_stencil, 
					clear_stencil_flags, 
					desc.depth, 
					desc.stencil, 
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

			curr_vertex_buffer_hash = 0;
			count = Min(count, MAX_VERTEX_STREAMS);
			for(u32 i =0; i < MAX_VERTEX_STREAMS; ++i)
			{
				bool changed = false;
				auto buffer = i < count ? buffers[i] : nullptr;

				if(buffer)
				{
					CombineHash(curr_vertex_buffer_hash, MakeHash(buffer));
					const auto& elements = buffer->GetElements();
					// Check if buffer has per-instance data
					const auto has_instance_data = elements.Size() && elements[0].perInstance_;
					const auto offset = has_instance_data ? instance_offset * buffer->GetVertexSize() : 0;

					if(buffer != vertex_buffers_[i])
						dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::vertex_buffer);
					vertex_buffers_[i] = buffer;
					vertex_offsets_[i] = instance_offset;
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

		WeakPtr<RenderSurface> depth_stencil_;
		ea::queue<ShaderParameterUpdateDesc> params_2_update_;

		// arrays
		ea::array<WeakPtr<RenderSurface>, MAX_RENDERTARGETS> render_targets_;
		ea::array<ShaderResourceTextureDesc, MAX_TEXTURE_UNITS> textures_;
		ea::array<SharedPtr<VertexBuffer>, MAX_VERTEX_STREAMS> vertex_buffers_;
		ea::array<u64, MAX_VERTEX_STREAMS> vertex_offsets_;
		SharedPtr<IndexBuffer> index_buffer_;

		SharedPtr<VertexDeclaration> vertex_declaration_;
		SharedPtr<ShaderProgram> shader_program_;

		bool enable_clip_planes_{};
		Vector4 clip_plane_{};

		IntRect viewport_{};
		u32 dirty_flags_{};

		u32 curr_vertx_decl_hash{};
		u32 curr_vertex_buffer_hash{};
		u32 curr_pipeline_hash_{};

		u32 primitive_count_;
		u32 num_batches_;
	};

	Atomic::IDrawCommand* graphics_create_command(Atomic::Graphics* graphics)
	{
		return new DrawCommandImpl(graphics, graphics->GetImpl()->GetDeviceContext());
	}

}