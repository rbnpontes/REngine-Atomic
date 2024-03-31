#include "./RenderCommand.h"

#include "./DiligentUtils.h"

namespace REngine
{
    static Diligent::ITextureView* s_tmp_render_targets[Atomic::MAX_RENDERTARGETS];
    static Diligent::IBuffer* s_tmp_vertex_buffers[Atomic::MAX_VERTEX_STREAMS];

    void render_command_process(const RenderCommandProcessDesc& desc, RenderCommandState& state)
    {
        const auto graphics = desc.graphics;
        const auto driver = desc.driver;
        const auto context = driver->GetDeviceContext();

        // resolve render target and depth stencil formats
        const auto num_rts = Atomic::Min(state.num_rts, Atomic::MAX_RENDERTARGETS);
        state.pipeline_state_info.output.num_rts = num_rts;

        for (unsigned i = 0; i < num_rts; ++i)
        {
            assert(state.render_targets[i]);
            const auto rt_format = state.render_targets[i]->GetDesc().Format;
            if (rt_format != state.pipeline_state_info.output.render_target_formats[i])
                state.dirty_state |= static_cast<unsigned>(RenderCommandDirtyState::pipeline);
            state.pipeline_state_info.output.render_target_formats[i] = rt_format;
            s_tmp_render_targets[i] = state.render_targets[i];
        }

        auto depth_fmt = Diligent::TEX_FORMAT_UNKNOWN;
        if (state.depth_stencil)
            depth_fmt = state.depth_stencil->GetDesc().Format;

        if (depth_fmt != state.pipeline_state_info.output.depth_stencil_format)
            state.dirty_state |= static_cast<unsigned>(RenderCommandDirtyState::pipeline);
        state.pipeline_state_info.output.depth_stencil_format = depth_fmt;

        if(state.skip_flags & static_cast<unsigned>(RenderCommandSkipFlags::pipeline_build))
            state.skip_flags ^= static_cast<unsigned>(RenderCommandSkipFlags::pipeline_build);
        // build pipeline if is necessary
        else if ((state.dirty_state & static_cast<unsigned>(RenderCommandDirtyState::pipeline)) != 0)
        {
            auto pipeline_hash = state.pipeline_state_info.ToHash();
            if (pipeline_hash != state.pipeline_hash || state.pipeline_state == nullptr)
            {
                state.pipeline_hash = pipeline_hash;
                state.pipeline_state = pipeline_state_builder_acquire(driver, state.pipeline_state_info, pipeline_hash);
            }

            if(pipeline_hash != 0 && state.pipeline_state)
                state.dirty_state ^= static_cast<unsigned>(RenderCommandDirtyState::pipeline);
        }

        // bind textures
        Atomic::HashMap<Atomic::String, Diligent::IDeviceObject*> resources;
        if ((state.dirty_state & static_cast<unsigned>(RenderCommandDirtyState::textures)) != 0 && state.pipeline_state)
        {
            for (const auto& it : state.textures)
            {
                assert(!it.first_.Empty() && "It seems binded texture slot name is empty.");
                if (it.second_)
                    resources[it.first_] = it.second_;
            }

            state.dirty_state ^= static_cast<unsigned>(RenderCommandDirtyState::textures);
        }

        if(state.skip_flags & static_cast<unsigned>(RenderCommandSkipFlags::srb_build))
            state.skip_flags ^= static_cast<unsigned>(RenderCommandSkipFlags::srb_build);
        // build shader resource binding if is necessary
        else if(resources.Size() > 0 && state.pipeline_state)
        {
            ShaderResourceBindingCreateDesc srb_desc;
            srb_desc.resources = &resources;
            srb_desc.driver = driver;
            srb_desc.pipeline_hash = state.pipeline_hash;
            state.shader_resource_binding = pipeline_state_builder_get_or_create_srb(srb_desc);
        }

        // bind render targets if is necessary
        context->SetRenderTargets(num_rts, s_tmp_render_targets, state.depth_stencil,
                                    Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        /*if (state.dirty_state & static_cast<unsigned>(RenderCommandDirtyState::render_targets) || state.dirty_state & static_cast<unsigned>(RenderCommandDirtyState::depth_stencil))
        {
            state.dirty_state ^= static_cast<unsigned>(RenderCommandDirtyState::render_targets);
            state.dirty_state ^= static_cast<unsigned>(RenderCommandDirtyState::depth_stencil);
        }*/

        if(state.dirty_state & static_cast<unsigned>(RenderCommandDirtyState::viewport))
        {
            const auto rect = state.viewport;
            Diligent::Viewport viewport;
            viewport.TopLeftX = static_cast<float>(rect.left_);
            viewport.TopLeftY = static_cast<float>(rect.top_);
            viewport.Width = static_cast<float>(rect.right_ - rect.left_);
            viewport.Height = static_cast<float>(rect.bottom_ - rect.top_);
            viewport.MinDepth = 0.0f;
            viewport.MaxDepth = 1.0f;
            context->SetViewports(1, &viewport, graphics->GetWidth(), graphics->GetHeight());

            state.dirty_state ^= static_cast<unsigned>(RenderCommandDirtyState::viewport);
        }

        if(state.dirty_state & static_cast<unsigned>(RenderCommandDirtyState::scissor) && 
            state.pipeline_state_info.scissor_test_enabled)
        {
            const auto rect = state.scissor;
            const Diligent::Rect scissor = { rect.left_, rect.top_, rect.right_, rect.bottom_};
            context->SetScissorRects(1, &scissor, graphics->GetWidth(), graphics->GetHeight());

            state.dirty_state ^= static_cast<unsigned>(RenderCommandDirtyState::scissor);
        }
        if(state.dirty_state & static_cast<unsigned>(RenderCommandDirtyState::vertex_buffer))
        {
            unsigned next_idx =0;
            for(const auto& buffer : state.vertex_buffers)
            {
                if(!buffer)
                    continue;
                s_tmp_vertex_buffers[next_idx] = buffer;
                ++next_idx;
            }

            if(next_idx > 0)
                context->SetVertexBuffers(
                    0,
                    next_idx,
                    s_tmp_vertex_buffers,
                    state.vertex_offsets,
                    Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            state.dirty_state ^= static_cast<unsigned>(RenderCommandDirtyState::vertex_buffer);
        }

        context->SetIndexBuffer(state.index_buffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        if(state.pipeline_state)
            context->SetPipelineState(state.pipeline_state);
        if(state.shader_resource_binding)
            context->CommitShaderResources(state.shader_resource_binding, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }

    void render_command_reset(const Atomic::Graphics* graphics, RenderCommandState& state)
    {
        for (unsigned i = 0; i < Atomic::MAX_VERTEX_STREAMS; ++i)
        {
            state.vertex_buffers[i] = nullptr;
            state.vertex_offsets[i] = 0;
            s_tmp_vertex_buffers[i] = nullptr;
        }
        state.index_buffer = nullptr;

        for(uint32_t i =0; i < Atomic::MAX_RENDERTARGETS; ++i)
        {
	        state.render_targets[i] = nullptr;
			s_tmp_render_targets[i] = nullptr;
        }
        state.depth_stencil = nullptr;

        state.pipeline_hash = 0;
        state.pipeline_state = nullptr;
        state.pipeline_state_info = {};
        state.shader_resource_binding = nullptr;

        if(graphics->IsInitialized())
            state.viewport = Atomic::IntRect(0, 0, graphics->GetWidth(), graphics->GetHeight());
        else
            state.viewport = Atomic::IntRect::ZERO;
        state.scissor = Atomic::IntRect::ZERO;
        state.stencil_ref = 0;

        for (auto it : state.textures)
            it.second_ = nullptr;

        state.shader_program = {};

        state.dirty_state = static_cast<unsigned>(RenderCommandDirtyState::all);
    }

    void render_command_clear(const RenderCommandClearDesc& desc)
    {
        const auto driver = desc.driver;
        const auto context = driver->GetDeviceContext();

        if(desc.flags & Atomic::CLEAR_COLOR)
            context->ClearRenderTarget(desc.render_target, desc.clear_color.Data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        if(desc.flags & (Atomic::CLEAR_DEPTH | Atomic::CLEAR_STENCIL) && desc.depth_stencil)
            context->ClearDepthStencil(desc.depth_stencil,
                desc.clear_stencil_flags,
                desc.clear_depth,
                desc.clear_stencil,
                Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }
}
