#pragma once
#include "./DriverInstance.h"
#include "../Graphics/GraphicsDefs.h"
#include "../Container/Str.h"
#include "../Container/Vector.h"

#include <DiligentCore/Graphics/GraphicsEngine/interface/PipelineState.h>
#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>

#include "Math/MathDefs.h"

namespace REngine
{
    struct InputLayoutElementDesc
    {
        unsigned input_index{};
        unsigned buffer_index{};
        unsigned buffer_stride{};
        unsigned element_offset{};
        unsigned instance_step_rate{};
        Atomic::VertexElementType element_type;
    };

    struct InputLayoutDesc
    {
        unsigned num_elements{};
        InputLayoutElementDesc elements[Diligent::MAX_LAYOUT_ELEMENTS]{};
    };
    
    struct PipelineStateOutputDesc
    {
        Diligent::TEXTURE_FORMAT depth_stencil_format{};
        uint8_t num_rts{0};
        Diligent::TEXTURE_FORMAT render_target_formats[Atomic::MAX_RENDERTARGETS]{};
        uint8_t multi_sample{1};
    };

    struct SamplerDesc
    {
        Atomic::TextureFilterMode filter_mode{Atomic::FILTER_DEFAULT};
        uint8_t anisotropy{};
        bool shadow_compare{};
        Atomic::TextureAddressMode address_u{Atomic::ADDRESS_WRAP};
        Atomic::TextureAddressMode address_v{Atomic::ADDRESS_WRAP};
        Atomic::TextureAddressMode address_w{Atomic::ADDRESS_WRAP};
    };
    
    struct ImmutableSamplersDesc
    {
        Atomic::String name{};
        SamplerDesc sampler;
    };
    
    struct PipelineStateInfo
    {
        Atomic::String debug_name{};

        // BEGIN BLEND STATE
        bool color_write_enabled{true};
        Atomic::BlendMode blend_mode{Atomic::BLEND_REPLACE};
        bool alpha_to_coverage_enabled{false};
        // END BLEND_STATE

        // BEGIN RASTERIZER STATE
        Atomic::FillMode fill_mode{Atomic::FILL_SOLID};
        Atomic::CullMode cull_mode{Atomic::CULL_CCW};
        float constant_depth_bias{0.f};
        float slope_scaled_depth_bias{0.f};
        bool scissor_test_enabled{false};
        bool line_anti_alias{false};
        // END RASTERIZER STATE

        // BEGIN DEPTH STENCIL STATE
        bool depth_write_enabled{true};
        bool stencil_test_enabled{false};
        Atomic::CompareMode depth_cmp_function{Atomic::CMP_LESSEQUAL};
        Atomic::CompareMode stencil_cmp_function{Atomic::CMP_ALWAYS};
        Atomic::StencilOp stencil_op_on_passed{Atomic::OP_KEEP};
        Atomic::StencilOp stencil_op_on_stencil_failed{Atomic::OP_KEEP};
        Atomic::StencilOp stencil_op_depth_failed{Atomic::OP_KEEP};
        uint8_t stencil_cmp_mask{255};
        uint8_t stencil_write_mask{255};
        // END DEPTH STENCIL STATE

        InputLayoutDesc input_layout;
        Atomic::PrimitiveType primitive_type{Atomic::TRIANGLE_LIST};
        PipelineStateOutputDesc output;

        uint8_t num_samplers{0};
        ImmutableSamplersDesc immutable_samplers[Atomic::MAX_IMMUTABLE_SAMPLERS]{};

        bool read_only_depth{false};

        Diligent::RefCntAutoPtr<Diligent::IShader> vs_shader{};
        Diligent::RefCntAutoPtr<Diligent::IShader> ps_shader{};
        Diligent::RefCntAutoPtr<Diligent::IShader> ds_shader{};
        Diligent::RefCntAutoPtr<Diligent::IShader> hs_shader{};
        Diligent::RefCntAutoPtr<Diligent::IShader> gs_shader{};
    };

    /**
     * \brief build pipeline state according pipeline state info.
     * \param driver driver instance
     * \param info pipeline basic description
     * \param hash output pipeline hash
     * \return 
     */
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> pipeline_state_builder_acquire(DriverInstance* driver, const PipelineStateInfo& info,
                                                             unsigned& hash);
    /**
     * \brief get cached pipeline state from pipeline hash
     * \param pipeline_hash 
     * \return 
     */
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> pipeline_state_builder_get(const unsigned pipeline_hash);
    /**
     * \brief release cached pipeline states
     */
    void pipeline_state_builder_release();
    /**
     * \brief helper function to generate hash from pipeline state info
     * \param info 
     * \return 
     */
    unsigned pipeline_state_builder_build_hash(const PipelineStateInfo& info);
}
