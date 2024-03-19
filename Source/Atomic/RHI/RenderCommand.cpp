#include "./RenderCommand.h"

namespace REngine
{
    static Diligent::ITextureView* s_tmp_render_targets[Atomic::MAX_RENDERTARGETS];

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

        // build pipeline if is necessary
        if ((state.dirty_state & static_cast<unsigned>(RenderCommandDirtyState::pipeline)) != 0)
        {
            auto pipeline_hash = pipeline_state_builder_build_hash(state.pipeline_state_info);
            if (pipeline_hash != state.pipeline_hash || state.pipeline_state == nullptr)
            {
                state.pipeline_hash = pipeline_hash;
                state.pipeline_state = pipeline_state_builder_acquire(driver, state.pipeline_state_info, pipeline_hash);
            }

            state.dirty_state ^= static_cast<unsigned>(RenderCommandDirtyState::pipeline);
        }

        // bind textures
        Atomic::HashMap<Atomic::String, Diligent::IDeviceObject*> resources;
        if ((state.dirty_state & static_cast<unsigned>(RenderCommandDirtyState::textures)) != 0)
        {
            for (const auto& it : state.textures)
            {
                if (it.second_)
                    resources[it.first_] = it.second_;
            }

            state.dirty_state ^= static_cast<unsigned>(RenderCommandDirtyState::textures);
        }

        // bind constant buffers
        if ((state.dirty_state & static_cast<unsigned>(RenderCommandDirtyState::constant_buffers)) != 0)
        {
            Atomic::HashMap<Atomic::String, Diligent::RefCntAutoPtr<Diligent::IBuffer>> buffers[] = {
                state.vs_constant_buffers,
                state.ps_constant_buffers,
            };

            for (const auto& buffer : buffers)
            {
                for(const auto& it : buffer)
                {
                    if(it.second_)
                        resources[it.first_] = it.second_;
                }
            }

            state.dirty_state ^= static_cast<unsigned>(RenderCommandDirtyState::constant_buffers);
        }

        // build shader resource binding if is necessary
        if(resources.Size() > 0)
            state.shader_resource_binding = pipeline_state_builder_get_or_create_srb(state.pipeline_hash, resources);

        // bind render targets if is necessary
        if (state.dirty_state & static_cast<unsigned>(RenderCommandDirtyState::render_targets) || state.dirty_state & static_cast<unsigned>(RenderCommandDirtyState::depth_stencil))
        {
            context->SetRenderTargets(num_rts, s_tmp_render_targets, state.depth_stencil,
                                      Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            state.dirty_state ^= static_cast<unsigned>(RenderCommandDirtyState::render_targets);
            state.dirty_state ^= static_cast<unsigned>(RenderCommandDirtyState::depth_stencil);
        }

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

        if(state.dirty_state & static_cast<unsigned>(RenderCommandDirtyState::scissor))
        {
            const auto rect = state.scissor;
            const Diligent::Rect scissor = { rect.left_, rect.top_, rect.right_, rect.bottom_};
            context->SetScissorRects(1, &scissor, graphics->GetWidth(), graphics->GetHeight());

            state.dirty_state ^= static_cast<unsigned>(RenderCommandDirtyState::scissor);
        }
    }

    void render_command_reset(const Atomic::Graphics* graphics, RenderCommandState& state)
    {
        for (unsigned i = 0; i < Atomic::MAX_VERTEX_STREAMS; ++i)
        {
            state.vertex_buffers[i] = nullptr;
            state.vertex_offsets[i] = state.vertex_sizes[i] = 0;
        }
        state.index_buffer = nullptr;

        for (auto& render_target : state.render_targets)
            render_target = nullptr;
        state.depth_stencil = nullptr;

        state.pipeline_hash = 0;
        state.pipeline_state = nullptr;
        state.pipeline_state_info = {};
        state.shader_resource_binding = nullptr;

        state.viewport = Atomic::IntRect(0, 0, graphics->GetWidth(), graphics->GetHeight());
        state.scissor = Atomic::IntRect::ZERO;
        state.stencil_ref = 0;

        for (auto it : state.textures)
            it.second_ = nullptr;
        for (auto it : state.vs_constant_buffers)
            it.second_ = nullptr;
        for (auto it : state.ps_constant_buffers)
            it.second_ = nullptr;

        state.dirty_state = static_cast<unsigned>(RenderCommandDirtyState::all);
    }
}
