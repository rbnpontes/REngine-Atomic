#pragma once
#include "./RHITypes.h"

#include <DiligentCore/Graphics/GraphicsEngine/interface/PipelineState.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/ShaderResourceBinding.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/Buffer.h>

namespace REngine
{
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
