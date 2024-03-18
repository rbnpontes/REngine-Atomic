#pragma once
#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/Buffer.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/PipelineState.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/ShaderResourceBinding.h>
#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>

#include "./PipelineStateBuilder.h"
#include "../Graphics/GraphicsDefs.h"
#include "../Math/Rect.h"

namespace REngine
{
    using namespace Diligent;
    struct GraphicsState
    {
        ISwapChain* current_swap_chain;
    };

    struct RenderCommandState
    {
        RefCntAutoPtr<IBuffer> vertex_buffers[Atomic::MAX_VERTEX_STREAMS]{};
        uint64_t vertex_sizes[Atomic::MAX_VERTEX_STREAMS]{};
        uint64_t vertex_offsets[Atomic::MAX_VERTEX_STREAMS]{};
        RefCntAutoPtr<IBuffer> index_buffer{nullptr};

        RefCntAutoPtr<ITextureView> render_targets[Atomic::MAX_RENDERTARGETS];
        RefCntAutoPtr<ITextureView> depth_stencil{nullptr};

        unsigned pipeline_hash;
        RefCntAutoPtr<IPipelineState> pipeline_state{nullptr};
        RefCntAutoPtr<IShaderResourceBinding> shader_resource_binding{nullptr};

        PipelineStateInfo pipeline_state_info{};

        Atomic::IntRect viewport{Atomic::IntRect::ZERO};
        Atomic::IntRect scissor{Atomic::IntRect::ZERO};
        uint8_t stencil_ref{};

        // NOTE: idk if this is really necessary
        bool use_clip_plane{false};
    };

    const GraphicsState& graphics_state_get();
    void graphics_state_set(const GraphicsState state);

    const RenderCommandState& default_render_command_get();
    void default_render_command_set(const RenderCommandState state);

    
}
