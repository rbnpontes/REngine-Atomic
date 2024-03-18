#pragma once
#include "../Graphics/GraphicsDefs.h"
#include "./PipelineStateBuilder.h"
#include "./DriverInstance.h"
#include "../Math/Rect.h"
#include "../Graphics/Graphics.h"

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/Buffer.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/PipelineState.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/ShaderResourceBinding.h>


namespace REngine
{
    struct RenderCommandState
    {
        Diligent::RefCntAutoPtr<Diligent::IBuffer> vertex_buffers[Atomic::MAX_VERTEX_STREAMS]{};
        uint64_t vertex_sizes[Atomic::MAX_VERTEX_STREAMS]{};
        uint64_t vertex_offsets[Atomic::MAX_VERTEX_STREAMS]{};
        Diligent::RefCntAutoPtr<Diligent::IBuffer> index_buffer{nullptr};

        uint8_t num_rts{0};
        Diligent::RefCntAutoPtr<Diligent::ITextureView> render_targets[Atomic::MAX_RENDERTARGETS];
        Diligent::RefCntAutoPtr<Diligent::ITextureView> depth_stencil{nullptr};

        unsigned pipeline_hash;
        Diligent::RefCntAutoPtr<Diligent::IPipelineState> pipeline_state{nullptr};
        Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> shader_resource_binding{nullptr};

        PipelineStateInfo pipeline_state_info{};

        Atomic::IntRect viewport{Atomic::IntRect::ZERO};
        Atomic::IntRect scissor{Atomic::IntRect::ZERO};
        uint8_t stencil_ref{};

        // NOTE: idk if this is really necessary
        bool use_clip_plane{false};
    };

    struct RenderCommandProcessDesc
    {
        Atomic::Graphics* graphics;
        DriverInstance* driver;
    };

    /**
     * \brief Process Render Command State. This method creates PipelineState, Shader Resource Binding
     * And any other required items required to render.
     * \param desc 
     * \param state 
     */
    void render_command_process(const RenderCommandProcessDesc& desc, RenderCommandState& state);
    /**
     * \brief reset render command state
     * \param state 
     */
    void render_command_reset(const Atomic::Graphics* graphics, RenderCommandState& state);
}
