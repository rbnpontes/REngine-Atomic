#pragma once
#include "./DriverInstance.h"
#include "./PipelineStateBuilder.h"
#include "./ShaderProgram.h"

#include "../Graphics/GraphicsDefs.h"
#include "../Math/Rect.h"
#include "../Graphics/Graphics.h"

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
        shader_program = 1 << 4,
        viewport = 1 << 5,
        scissor = 1 << 6,
        vertex_buffer = 1 << 7,
        index_buffer = 1 << 8,
        vertex_decl = 1 << 9,
        srb = 1 << 10,
        all = render_targets | depth_stencil | pipeline
        | textures | shader_program | viewport
        | scissor | vertex_buffer | index_buffer | vertex_decl
        | srb
    };

    enum class RenderCommandSkipFlags : unsigned
    {
        none = 0x0,
        pipeline_build = 1 << 0,
        srb_build = 1 << 1
    };

    struct ShaderParameterUpdateDesc
    {
		Atomic::StringHash name{Atomic::StringHash::ZERO};
        Atomic::Variant value{Atomic::Variant::EMPTY};
	};
    
    struct RenderCommandState
    {
        Diligent::RefCntAutoPtr<Diligent::IBuffer> vertex_buffers[Atomic::MAX_VERTEX_STREAMS]{};
        u64 vertex_offsets[Atomic::MAX_VERTEX_STREAMS]{};
        Diligent::RefCntAutoPtr<Diligent::IBuffer> index_buffer{nullptr};

        u8 num_rts{0};
    	Diligent::ITextureView* render_targets[Atomic::MAX_RENDERTARGETS];
        Diligent::ITextureView* depth_stencil{nullptr};

        unsigned pipeline_hash;
        Diligent::RefCntAutoPtr<Diligent::IPipelineState> pipeline_state{nullptr};
        Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> shader_resource_binding{nullptr};

        PipelineStateInfo pipeline_state_info{};
        u32 vertex_decl_hash{0};

        Atomic::IntRect viewport{Atomic::IntRect::ZERO};
        Atomic::IntRect scissor{Atomic::IntRect::ZERO};
        u8 stencil_ref{0};

        ShaderResourceTextures textures{};

        Atomic::SharedPtr<ShaderProgram> shader_program{};

        Atomic::Vector<ShaderParameterUpdateDesc> shader_parameter_updates{};

        unsigned dirty_state{static_cast<unsigned>(RenderCommandDirtyState::all)};
        unsigned skip_flags{static_cast<unsigned>(RenderCommandSkipFlags::none)};
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
        u8 clear_stencil{0};
        Diligent::CLEAR_DEPTH_STENCIL_FLAGS clear_stencil_flags{Diligent::CLEAR_DEPTH_FLAG_NONE};
        unsigned flags{0};
    };

    /**
     * \brief Process Render Command State. This method creates PipelineState, Shader Resource Binding
     * And any other required items required to render.
     * \param desc 
     * \param state 
     */
    void render_command_process(const RenderCommandProcessDesc& desc, RenderCommandState* state);
    /**
     * \brief reset render command state
     * \param state 
     */
    void render_command_reset(const Atomic::Graphics* graphics, RenderCommandState* state);

    void render_command_clear(const RenderCommandClearDesc& desc);

    void render_command_update_params(const Atomic::Graphics* graphics, RenderCommandState* state);

    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const Atomic::Variant& value);
}
