#pragma once
#include "../Graphics/GraphicsDefs.h"
#include "./PipelineStateBuilder.h"
#include "./DriverInstance.h"
#include "../Math/Rect.h"
#include "../Graphics/Graphics.h"
#include "../Container/HashMap.h"

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/Buffer.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/PipelineState.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/ShaderResourceBinding.h>

namespace REngine
{
    enum class RenderCommandDirtyState : unsigned
    {
        none = 0x0,
        render_targets = 1 << 0,
        depth_stencil = 1 << 1,
        pipeline = 1 << 2,
        textures = 1 << 3,
        constant_buffers = 1 << 4,
        viewport = 1 << 5,
        scissor = 1 << 6,
        all = render_targets | depth_stencil | pipeline | textures | viewport | scissor
    };
    
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

        Atomic::HashMap<Atomic::String, Diligent::RefCntAutoPtr<Diligent::ITextureView>> textures{};
        Atomic::HashMap<Atomic::String, Diligent::RefCntAutoPtr<Diligent::IBuffer>> vs_constant_buffers{};
        Atomic::HashMap<Atomic::String, Diligent::RefCntAutoPtr<Diligent::IBuffer>> ps_constant_buffers{};

        unsigned dirty_state{static_cast<unsigned>(RenderCommandDirtyState::all)};
    };

    struct RenderCommandProcessDesc
    {
        Atomic::Graphics* graphics;
        DriverInstance* driver;
    };

    struct RenderCommandClearDesc
    {
        DriverInstance* driver{nullptr};
        Diligent::ITextureView* render_target{nullptr};
        Diligent::ITextureView* depth_stencil{nullptr};
        Atomic::Color clear_color{Atomic::Color::BLACK};
        float clear_depth{0};
        uint8_t clear_stencil{0};
        Diligent::CLEAR_DEPTH_STENCIL_FLAGS clear_stencil_flags{Diligent::CLEAR_DEPTH_FLAG_NONE};
        unsigned flags{0};
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

    void render_command_clear(const RenderCommandClearDesc& desc);
}
