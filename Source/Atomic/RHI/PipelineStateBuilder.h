#pragma once
#include "./DriverInstance.h"
#include "./RHITypes.h"
#include "../Container/Str.h"
#include "../Container/Ptr.h"
#include "../Graphics/ShaderVariation.h"

#include <DiligentCore/Graphics/GraphicsEngine/interface/PipelineState.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/ShaderResourceBinding.h>
#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>

#include "../Container/HashMap.h"

namespace REngine
{
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
        u8 stencil_cmp_mask{255};
        u8 stencil_write_mask{255};
        // END DEPTH STENCIL STATE

        InputLayoutDesc input_layout{};
        Atomic::PrimitiveType primitive_type{Atomic::TRIANGLE_LIST};
        PipelineStateOutputDesc output{};

        u8 num_samplers{0};
        ImmutableSamplersDesc immutable_samplers[Atomic::MAX_IMMUTABLE_SAMPLERS]{};

        bool read_only_depth{false};

        Atomic::SharedPtr<Atomic::ShaderVariation> vs_shader{};
        Atomic::SharedPtr<Atomic::ShaderVariation> ps_shader{};
        Atomic::SharedPtr<Atomic::ShaderVariation> ds_shader{};
        Atomic::SharedPtr<Atomic::ShaderVariation> hs_shader{};
        Atomic::SharedPtr<Atomic::ShaderVariation> gs_shader{};

        uint32_t ToHash() const
        {
            uint32_t hash = Atomic::StringHash::Calculate(debug_name.CString());

            Atomic::CombineHash(hash, color_write_enabled ? 1u : 0u);
            Atomic::CombineHash(hash, static_cast<uint32_t>(blend_mode));
            Atomic::CombineHash(hash, alpha_to_coverage_enabled ? 1u : 0u);

            Atomic::CombineHash(hash, static_cast<uint32_t>(fill_mode));
            Atomic::CombineHash(hash, static_cast<uint32_t>(cull_mode));
            Atomic::CombineHash(hash, static_cast<uint32_t>(constant_depth_bias));
            Atomic::CombineHash(hash, static_cast<uint32_t>(slope_scaled_depth_bias));
            Atomic::CombineHash(hash, scissor_test_enabled ? 1u : 0u);
            Atomic::CombineHash(hash, line_anti_alias ? 1u : 0u);

            Atomic::CombineHash(hash, depth_write_enabled ? 1u : 0u);
            Atomic::CombineHash(hash, stencil_test_enabled ? 1u : 0u);
            Atomic::CombineHash(hash, static_cast<uint32_t>(depth_cmp_function));
            Atomic::CombineHash(hash, static_cast<uint32_t>(stencil_cmp_function));
            Atomic::CombineHash(hash, static_cast<uint32_t>(stencil_op_on_passed));
            Atomic::CombineHash(hash, static_cast<uint32_t>(stencil_op_on_stencil_failed));
            Atomic::CombineHash(hash, static_cast<uint32_t>(stencil_op_depth_failed));
            Atomic::CombineHash(hash, stencil_cmp_mask);
            Atomic::CombineHash(hash, stencil_write_mask);

            Atomic::CombineHash(hash, input_layout.ToHash());
            Atomic::CombineHash(hash, static_cast<uint32_t>(primitive_type));
            Atomic::CombineHash(hash, output.ToHash());

            Atomic::CombineHash(hash, num_samplers);
            for(uint8_t i = 0; i < num_samplers; ++i)
            	Atomic::CombineHash(hash, immutable_samplers[i].ToHash());

            if(vs_shader)
				Atomic::CombineHash(hash, vs_shader->ToHash());
            if(ps_shader)
                Atomic::CombineHash(hash, ps_shader->ToHash());
            if(ds_shader)
				Atomic::CombineHash(hash, ds_shader->ToHash());
            if(hs_shader)
                Atomic::CombineHash(hash, hs_shader->ToHash());
            if(gs_shader)
				Atomic::CombineHash(hash, gs_shader->ToHash());

            return hash;
        }
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

    uint32_t pipeline_state_builder_items_count();

    struct ShaderResourceBindingCreateDesc
    {
        DriverInstance* driver{nullptr};
        u32 pipeline_hash{0};
        ShaderResourceTextures* resources{nullptr};
    };
    /**
     * \brief get or create a shader resource binding from an pipeline hash
     * \return 
     */
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> pipeline_state_builder_get_or_create_srb(const ShaderResourceBindingCreateDesc& desc);

    void srb_cache_update_default_cbuffers(const Atomic::ShaderType type, const Atomic::ShaderParameterGroup grp, Diligent::IBuffer* cbuffer);
    /**
     * \brief release cached shader resource bindings
     */
    void srb_cache_release();

    uint32_t srb_cache_items_count();
}
