#include "./RenderCommand.h"

namespace REngine
{
    void render_command_process(const RenderCommandProcessDesc& desc, RenderCommandState& state)
    {
        const auto graphics = desc.graphics;
        const auto driver = desc.driver;

        // resolve render target and depth stencil formats
        const auto num_rts = Atomic::Min(state.num_rts, Atomic::MAX_RENDERTARGETS);
        state.pipeline_state_info.output.num_rts = num_rts;
        for(unsigned i = 0; i < num_rts; ++i)
        {
            assert(state.render_targets[i]);
            state.pipeline_state_info.output.render_target_formats[i]
                = state.render_targets[i]->GetDesc().Format; 
        }
        
        if(state.depth_stencil)
            state.pipeline_state_info.output.depth_stencil_format = state.depth_stencil->GetDesc().Format;
        
        const auto default_filter_mode = graphics->GetDefaultTextureFilterMode();
        const auto default_anisotropy = graphics->GetDefaultTextureAnisotropy(); 
        // check if any sampler is using default filter or default anisotropy
        for(unsigned i = 0; i < state.pipeline_state_info.num_samplers; ++i)
        {
            auto& sampler = state.pipeline_state_info.immutable_samplers[i].sampler;
            if(sampler.filter_mode == Atomic::FILTER_DEFAULT)
                sampler.filter_mode = default_filter_mode;
            if(sampler.anisotropy == 0)
                sampler.anisotropy = static_cast<uint8_t>(default_anisotropy);
        }

        auto pipeline_hash = pipeline_state_builder_build_hash(state.pipeline_state_info);
        if(pipeline_hash != state.pipeline_hash || state.pipeline_state == nullptr)
        {
            state.pipeline_hash = pipeline_hash;
            state.pipeline_state = pipeline_state_builder_acquire(driver, state.pipeline_state_info, pipeline_hash);
        }
    }

}
