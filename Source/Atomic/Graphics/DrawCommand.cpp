#include "DrawCommand.h"

#include "Geometry.h"
#include "Model.h"
#include "Renderer.h"
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
	static Diligent::IPipelineState* s_pipeline = nullptr;
	static ea::array<Diligent::IBuffer*, MAX_VERTEX_STREAMS> s_vertex_buffers = {};
	static ea::array<u64, MAX_VERTEX_STREAMS> s_vertex_offsets = {};
	static u8 s_num_vertex_buffers = 0;

	class DrawCommandImpl : public IDrawCommand
	{
	public:
		DrawCommandImpl(Graphics* graphics, const Diligent::RefCntAutoPtr<Diligent::IDeviceContext>& context) :
			context_(context),
			graphics_(graphics),
			viewport_(IntRect::ZERO),
			dirty_flags_(static_cast<u32>(RenderCommandDirtyState::all))
		{
			pipeline_info_ = new PipelineStateInfo();
		}
		~DrawCommandImpl()
		{
			delete pipeline_info_;
		}

		void Reset() override
		{
			*pipeline_info_ = PipelineStateInfo{};
			pipeline_info_->output.render_target_formats.fill(Diligent::TEX_FORMAT_UNKNOWN);

			const auto wnd_size = graphics_->GetRenderTargetDimensions();
			viewport_ = IntRect(0, 0, wnd_size.x_, wnd_size.y_);
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
		void PrepareClear()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareClear);
			const auto changed = PrepareDepthStencil() || PrepareDepthStencil();
			if (changed)
				BoundRenderTargets();
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
			u32 pipeline_hash = 0;
			ShaderResourceTextures textures_dummy = {};

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
			draw_attribs.IndexType = GetVertexSize(geometry->GetIndexBuffer()->GetIndexSize());
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
				s_vertex_offsets.data(),
				RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
				SET_VERTEX_BUFFERS_FLAG_RESET);
			context_->SetIndexBuffer(nullptr, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			context_->SetStencilRef(desc.stencil);
			context_->SetPipelineState(pipeline);
			context_->CommitShaderResources(srb, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			context_->DrawIndexed(draw_attribs);
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
		static Diligent::VALUE_TYPE GetVertexSize(u32 size)
		{
			return size == sizeof(u16) ? Diligent::VT_UINT16 : Diligent::VT_UINT32;
		}

		Graphics* graphics_;
		Diligent::RefCntAutoPtr<Diligent::IDeviceContext> context_;
		PipelineStateInfo* pipeline_info_;

		ea::array<WeakPtr<RenderSurface>, MAX_RENDERTARGETS> render_targets_;
		WeakPtr<RenderSurface> depth_stencil_;

		IntRect viewport_{};
		u32 dirty_flags_{};
	};

	Atomic::IDrawCommand* graphics_create_command(Atomic::Graphics* graphics)
	{
		return new DrawCommandImpl(graphics);
	}

}