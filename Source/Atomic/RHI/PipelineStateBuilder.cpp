#include "./PipelineStateBuilder.h"
#include "./DiligentUtils.h"
#include "../Container/HashMap.h"
#include "../Container/Hash.h"
#include "../IO/Log.h"

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>

namespace REngine
{
    static Atomic::HashMap<unsigned, Diligent::RefCntAutoPtr<Diligent::IPipelineState>> s_pipelines;
    static Atomic::HashMap<unsigned, Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>> s_srb;

    static uint8_t s_num_components_tbl[] = {
        1,
        1,
        2,
        3,
        4,
        4,
        4
    };
    static Diligent::VALUE_TYPE s_value_types_tbl[] = {
        Diligent::VT_INT32,
        Diligent::VT_FLOAT32,
        Diligent::VT_FLOAT32,
        Diligent::VT_FLOAT32,
        Diligent::VT_FLOAT32,
        Diligent::VT_UINT8,
        Diligent::VT_UINT8
    };
    static bool s_is_normalized_tbl[] = {
        false,
        false,
        false,
        false,
        false,
        false,
        true
    };
    static Diligent::FILTER_TYPE s_min_mag_filters_tbl[][2] = {
        {Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_COMPARISON_POINT},
        {Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_COMPARISON_LINEAR},
        {Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_COMPARISON_LINEAR},
        {Diligent::FILTER_TYPE_ANISOTROPIC, Diligent::FILTER_TYPE_COMPARISON_ANISOTROPIC},
        {Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_COMPARISON_POINT},
    };
    static Diligent::FILTER_TYPE s_mip_filters_tbl[][2] = {
        {Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_COMPARISON_POINT},
        {Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_COMPARISON_POINT},
        {Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_COMPARISON_LINEAR},
        {Diligent::FILTER_TYPE_ANISOTROPIC, Diligent::FILTER_TYPE_COMPARISON_ANISOTROPIC},
        {Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR},
    };
    static Diligent::TEXTURE_ADDRESS_MODE s_address_modes_tbl[] = {
        Diligent::TEXTURE_ADDRESS_WRAP,
        Diligent::TEXTURE_ADDRESS_MIRROR,
        Diligent::TEXTURE_ADDRESS_CLAMP,
        Diligent::TEXTURE_ADDRESS_BORDER
    };
    static Diligent::PRIMITIVE_TOPOLOGY s_primitive_topologies_tbl[] = {
        Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        Diligent::PRIMITIVE_TOPOLOGY_LINE_LIST,
        Diligent::PRIMITIVE_TOPOLOGY_POINT_LIST,
        Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        Diligent::PRIMITIVE_TOPOLOGY_LINE_STRIP
    };
    static Diligent::COMPARISON_FUNCTION s_comparison_functions_tbl[] = {
        Diligent::COMPARISON_FUNC_ALWAYS,
        Diligent::COMPARISON_FUNC_EQUAL,
        Diligent::COMPARISON_FUNC_NOT_EQUAL,
        Diligent::COMPARISON_FUNC_LESS,
        Diligent::COMPARISON_FUNC_LESS_EQUAL,
        Diligent::COMPARISON_FUNC_GREATER,
        Diligent::COMPARISON_FUNC_GREATER_EQUAL
    };
    static bool s_is_blend_enabled_tbl[] = {
        false,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
    };
    static Diligent::BLEND_FACTOR s_source_blends_tbl[] = {
        Diligent::BLEND_FACTOR_ONE,
        Diligent::BLEND_FACTOR_ONE,
        Diligent::BLEND_FACTOR_DEST_COLOR,
        Diligent::BLEND_FACTOR_SRC_ALPHA,
        Diligent::BLEND_FACTOR_SRC_ALPHA,
        Diligent::BLEND_FACTOR_ONE,
        Diligent::BLEND_FACTOR_INV_DEST_ALPHA,
        Diligent::BLEND_FACTOR_ONE,
        Diligent::BLEND_FACTOR_SRC_ALPHA,
        Diligent::BLEND_FACTOR_SRC_ALPHA,
    };
    static Diligent::BLEND_FACTOR s_dest_blends_tbl[] = {
        Diligent::BLEND_FACTOR_ZERO,
        Diligent::BLEND_FACTOR_ONE,
        Diligent::BLEND_FACTOR_ZERO,
        Diligent::BLEND_FACTOR_INV_SRC_ALPHA,
        Diligent::BLEND_FACTOR_ONE,
        Diligent::BLEND_FACTOR_INV_SRC_ALPHA,
        Diligent::BLEND_FACTOR_DEST_ALPHA,
        Diligent::BLEND_FACTOR_ONE,
        Diligent::BLEND_FACTOR_ONE,
        Diligent::BLEND_FACTOR_INV_SRC_ALPHA,
    };
    static Diligent::BLEND_FACTOR s_source_alpha_blends_tbl[] = {
        Diligent::BLEND_FACTOR_ONE,
        Diligent::BLEND_FACTOR_ONE,
        Diligent::BLEND_FACTOR_DEST_COLOR,
        Diligent::BLEND_FACTOR_SRC_ALPHA,
        Diligent::BLEND_FACTOR_SRC_ALPHA,
        Diligent::BLEND_FACTOR_ONE,
        Diligent::BLEND_FACTOR_INV_DEST_ALPHA,
        Diligent::BLEND_FACTOR_ONE,
        Diligent::BLEND_FACTOR_SRC_ALPHA,
        Diligent::BLEND_FACTOR_ZERO,
    };
    static Diligent::BLEND_FACTOR s_dest_alpha_blends_tbl[] = {
        Diligent::BLEND_FACTOR_ZERO,
        Diligent::BLEND_FACTOR_ONE,
        Diligent::BLEND_FACTOR_ZERO,
        Diligent::BLEND_FACTOR_INV_SRC_ALPHA,
        Diligent::BLEND_FACTOR_ONE,
        Diligent::BLEND_FACTOR_INV_SRC_ALPHA,
        Diligent::BLEND_FACTOR_DEST_ALPHA,
        Diligent::BLEND_FACTOR_ONE,
        Diligent::BLEND_FACTOR_ONE,
        Diligent::BLEND_FACTOR_INV_SRC_ALPHA,
    };
    static Diligent::BLEND_OPERATION s_blend_operations_tbl[] = {
        Diligent::BLEND_OPERATION_ADD,
        Diligent::BLEND_OPERATION_ADD,
        Diligent::BLEND_OPERATION_ADD,
        Diligent::BLEND_OPERATION_ADD,
        Diligent::BLEND_OPERATION_ADD,
        Diligent::BLEND_OPERATION_ADD,
        Diligent::BLEND_OPERATION_ADD,
        Diligent::BLEND_OPERATION_REV_SUBTRACT,
        Diligent::BLEND_OPERATION_REV_SUBTRACT,
        Diligent::BLEND_OPERATION_ADD,
    };
    static Diligent::STENCIL_OP s_stencil_operations_tbl[] = {
        Diligent::STENCIL_OP_KEEP,
        Diligent::STENCIL_OP_ZERO,
        Diligent::STENCIL_OP_REPLACE,
        Diligent::STENCIL_OP_INCR_WRAP,
        Diligent::STENCIL_OP_DECR_WRAP,
    };
    static Diligent::CULL_MODE s_cull_mode_tbl[] = {
        Diligent::CULL_MODE_NONE,
        Diligent::CULL_MODE_BACK,
        Diligent::CULL_MODE_FRONT
    };
    static Diligent::FILL_MODE s_fill_mode_tbl[] = {
        Diligent::FILL_MODE_SOLID,
        Diligent::FILL_MODE_WIREFRAME,
    };

    static Diligent::LayoutElement s_tmp_layout_elements[Diligent::MAX_LAYOUT_ELEMENTS] = {};
    static Diligent::ImmutableSamplerDesc s_tmp_immutable_samplers[Atomic::MAX_IMMUTABLE_SAMPLERS] = {};

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> pipeline_state_builder_acquire(
        DriverInstance* driver, const PipelineStateInfo& info,
        unsigned& hash)
    {
        if (hash == 0)
            hash = info.ToHash();

        // If hash matches from previous stored pipeline state
        // then return it instead.
        const auto pipeline_it = s_pipelines.Find(hash);
        if (pipeline_it != s_pipelines.End())
            return s_pipelines[hash];

        if(info.vs_shader == nullptr || info.ps_shader == nullptr)
        {
            ATOMIC_LOGWARNING("Vertex Shader and Pixel Shader is required to build pipeline state.");
            hash = 0;
            return {};
        }

        String name = info.debug_name;
        name.AppendWithFormat("#%d", hash);
        
        Diligent::GraphicsPipelineStateCreateInfo ci;
		ci.PSODesc.Name = name.CString();

		if (info.vs_shader && info.vs_shader->GetGPUObject())
			ci.pVS = info.vs_shader->GetGPUObject().Cast<Diligent::IShader>(Diligent::IID_Shader);
		if (info.ps_shader && info.ps_shader->GetGPUObject())
			ci.pPS = info.ps_shader->GetGPUObject().Cast<Diligent::IShader>(Diligent::IID_Shader);
		if (info.ds_shader && info.ds_shader->GetGPUObject())
			ci.pDS = info.ds_shader->GetGPUObject().Cast<Diligent::IShader>(Diligent::IID_Shader);
		if (info.hs_shader && info.hs_shader->GetGPUObject())
			ci.pHS = info.hs_shader->GetGPUObject().Cast<Diligent::IShader>(Diligent::IID_Shader);
		if (info.gs_shader && info.gs_shader->GetGPUObject())
			ci.pGS = info.gs_shader->GetGPUObject().Cast<Diligent::IShader>(Diligent::IID_Shader);

        for (unsigned i = 0; i < info.input_layout.num_elements; ++i)
        {
            Diligent::LayoutElement layout_element;
            auto element = info.input_layout.elements[i];
            layout_element.HLSLSemantic = "ATTRIB";
            layout_element.InputIndex = element.input_index;
            layout_element.RelativeOffset = element.element_offset;
            layout_element.NumComponents = s_num_components_tbl[element.element_type];
            layout_element.ValueType = s_value_types_tbl[element.element_type];
            layout_element.IsNormalized = s_is_normalized_tbl[element.element_type];
            layout_element.BufferSlot = element.buffer_index;
            layout_element.Stride = element.buffer_stride;
        	layout_element.Frequency = element.instance_step_rate != 0
                                           ? Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE
                                           : Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX;
            layout_element.InstanceDataStepRate = element.instance_step_rate;

            s_tmp_layout_elements[i] = layout_element;
        }

        ci.GraphicsPipeline.InputLayout.NumElements = info.input_layout.num_elements;
        ci.GraphicsPipeline.InputLayout.LayoutElements = s_tmp_layout_elements;

        for (unsigned i = 0; i < info.num_samplers; ++i)
        {
            Diligent::ImmutableSamplerDesc immutable_sampler;
            const auto& item = info.immutable_samplers[i];
            auto shadow_cmp = item.sampler.shadow_compare ? 1 : 0;

            immutable_sampler.SamplerOrTextureName = item.name;
            immutable_sampler.ShaderStages = Diligent::SHADER_TYPE_VS_PS;
            immutable_sampler.Desc.MinFilter = s_min_mag_filters_tbl[item.sampler.filter_mode][shadow_cmp];
            immutable_sampler.Desc.MagFilter = s_min_mag_filters_tbl[item.sampler.filter_mode][shadow_cmp];
            immutable_sampler.Desc.MipFilter = s_mip_filters_tbl[item.sampler.filter_mode][shadow_cmp];
            immutable_sampler.Desc.AddressU = s_address_modes_tbl[item.sampler.address_u];
            immutable_sampler.Desc.AddressV = s_address_modes_tbl[item.sampler.address_v];
            immutable_sampler.Desc.AddressW = s_address_modes_tbl[item.sampler.address_w];
            immutable_sampler.Desc.MaxAnisotropy = item.sampler.anisotropy;
            immutable_sampler.Desc.ComparisonFunc = Diligent::COMPARISON_FUNC_LESS_EQUAL;
            immutable_sampler.Desc.MinLOD = -Atomic::M_INFINITY;
            immutable_sampler.Desc.MaxLOD = Atomic::M_INFINITY;

            s_tmp_immutable_samplers[i] = immutable_sampler;
        }
        ci.PSODesc.ResourceLayout.NumImmutableSamplers = info.num_samplers;
        ci.PSODesc.ResourceLayout.ImmutableSamplers = s_tmp_immutable_samplers;

        ci.GraphicsPipeline.PrimitiveTopology = s_primitive_topologies_tbl[info.primitive_type];

        ci.GraphicsPipeline.NumRenderTargets = info.output.num_rts;
        for (unsigned i = 0; i < info.output.num_rts; ++i)
            ci.GraphicsPipeline.RTVFormats[i] = info.output.render_target_formats[i];
        ci.GraphicsPipeline.DSVFormat = info.output.depth_stencil_format;
        ci.GraphicsPipeline.SmplDesc.Count = info.output.multi_sample;

        ci.GraphicsPipeline.BlendDesc.AlphaToCoverageEnable = info.alpha_to_coverage_enabled;
        ci.GraphicsPipeline.BlendDesc.IndependentBlendEnable = false;

        const auto blend_mode = info.blend_mode;
        ci.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = s_is_blend_enabled_tbl[blend_mode];
        ci.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend = s_source_blends_tbl[blend_mode];
        ci.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend = s_dest_blends_tbl[blend_mode];
        ci.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOp = s_blend_operations_tbl[blend_mode];
        ci.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlendAlpha = s_source_alpha_blends_tbl[blend_mode];
        ci.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlendAlpha = s_dest_alpha_blends_tbl[blend_mode];
        ci.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOpAlpha = s_blend_operations_tbl[blend_mode];
        ci.GraphicsPipeline.BlendDesc.RenderTargets[0].RenderTargetWriteMask = info.color_write_enabled
            ? Diligent::COLOR_MASK_ALL
            : Diligent::COLOR_MASK_NONE;

        ci.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
        ci.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = info.depth_write_enabled;
        ci.GraphicsPipeline.DepthStencilDesc.DepthFunc = s_comparison_functions_tbl[info.depth_cmp_function];
        ci.GraphicsPipeline.DepthStencilDesc.StencilEnable = info.stencil_test_enabled;
        ci.GraphicsPipeline.DepthStencilDesc.StencilReadMask = info.stencil_cmp_mask;
        ci.GraphicsPipeline.DepthStencilDesc.StencilWriteMask = info.stencil_write_mask;
        ci.GraphicsPipeline.DepthStencilDesc.FrontFace.StencilFailOp = s_stencil_operations_tbl[info.
            stencil_op_on_stencil_failed];
        ci.GraphicsPipeline.DepthStencilDesc.FrontFace.StencilDepthFailOp = s_stencil_operations_tbl[info.
            stencil_op_depth_failed];
        ci.GraphicsPipeline.DepthStencilDesc.FrontFace.StencilPassOp = s_stencil_operations_tbl[info.
            stencil_op_on_passed];
        ci.GraphicsPipeline.DepthStencilDesc.FrontFace.StencilFunc = s_comparison_functions_tbl[info.
            stencil_cmp_function];
        ci.GraphicsPipeline.DepthStencilDesc.BackFace.StencilFailOp = s_stencil_operations_tbl[info.
            stencil_op_on_stencil_failed];
        ci.GraphicsPipeline.DepthStencilDesc.BackFace.StencilDepthFailOp = s_stencil_operations_tbl[info.
            stencil_op_depth_failed];
        ci.GraphicsPipeline.DepthStencilDesc.BackFace.StencilPassOp = s_stencil_operations_tbl[info.
            stencil_op_on_passed];
        ci.GraphicsPipeline.DepthStencilDesc.BackFace.StencilFunc = s_comparison_functions_tbl[info.
            stencil_cmp_function];

        unsigned depth_bits = 24;
        auto is_not_opengl = driver->GetBackend() != Atomic::GraphicsBackend::OpenGL;
        if (info.output.depth_stencil_format == Diligent::TEX_FORMAT_D16_UNORM)
            depth_bits = 16;
    	const int scaled_depth_bias = is_not_opengl
                                          ? static_cast<int>(info.constant_depth_bias * static_cast<float>((1 <<
                                              depth_bits)))
                                          : 0;

        ci.GraphicsPipeline.RasterizerDesc.FillMode = s_fill_mode_tbl[info.fill_mode];
        ci.GraphicsPipeline.RasterizerDesc.CullMode = s_cull_mode_tbl[info.cull_mode];
    	ci.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = false;
        ci.GraphicsPipeline.RasterizerDesc.DepthBias = scaled_depth_bias;
        ci.GraphicsPipeline.RasterizerDesc.DepthBiasClamp = M_INFINITY;
        ci.GraphicsPipeline.RasterizerDesc.SlopeScaledDepthBias = info.slope_scaled_depth_bias;
        ci.GraphicsPipeline.RasterizerDesc.DepthClipEnable = true;
        ci.GraphicsPipeline.RasterizerDesc.ScissorEnable = info.scissor_test_enabled;
        //ci.GraphicsPipeline.RasterizerDesc.ScissorEnable = false;
        ci.GraphicsPipeline.RasterizerDesc.AntialiasedLineEnable = is_not_opengl && info.line_anti_alias;

        ci.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC;

        Diligent::RefCntAutoPtr<Diligent::IPipelineState> result;
        driver->GetDevice()->CreatePipelineState(ci, &result);

        if(!result)
            ATOMIC_LOGERRORF("Failed to create pipeline state: %s.", name.CString());
        assert(result);

        s_pipelines[hash] = result;
        return result;
    }

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> pipeline_state_builder_get(const unsigned pipeline_hash)
    {
        const auto it = s_pipelines.Find(pipeline_hash);
        if (it == s_pipelines.End())
            return {};
        return it->second_;
    }

    void pipeline_state_builder_release()
    {
        s_pipelines.Clear();
        s_srb.Clear();
    }

    uint32_t pipeline_state_builder_items_count()
    {
	    return s_pipelines.Size();
	}

    unsigned pipeline_state_builder_build_hash(const PipelineStateInfo& info)
    {
        unsigned hash = Atomic::StringHash::Calculate(info.debug_name.CString());
        // Blend State
        Atomic::CombineHash(hash, info.color_write_enabled);
        Atomic::CombineHash(hash, info.blend_mode);
        Atomic::CombineHash(hash, info.alpha_to_coverage_enabled);
        // Rasterizer State
        Atomic::CombineHash(hash, info.fill_mode);
        Atomic::CombineHash(hash, info.cull_mode);
        Atomic::CombineHash(hash, static_cast<unsigned>(info.constant_depth_bias));
        Atomic::CombineHash(hash, static_cast<unsigned>(info.slope_scaled_depth_bias));
        Atomic::CombineHash(hash, info.line_anti_alias);
        // Depth Stencil
        Atomic::CombineHash(hash, info.depth_write_enabled);
        Atomic::CombineHash(hash, info.stencil_test_enabled);
        Atomic::CombineHash(hash, info.depth_cmp_function);
        Atomic::CombineHash(hash, info.stencil_cmp_function);
        Atomic::CombineHash(hash, info.stencil_op_on_passed);
        Atomic::CombineHash(hash, info.stencil_op_on_stencil_failed);
        Atomic::CombineHash(hash, info.stencil_op_depth_failed);
        Atomic::CombineHash(hash, info.stencil_cmp_mask);
        Atomic::CombineHash(hash, info.stencil_write_mask);
        // Input Layout
        for (unsigned i = 0; i < info.input_layout.num_elements; ++i)
            Atomic::CombineHash(hash, info.input_layout.elements[i].ToHash());
        // Primitive Type
        Atomic::CombineHash(hash, info.primitive_type);
        // Immutable Samplers
        for (unsigned i = 0; i < info.num_samplers; ++i)
            Atomic::CombineHash(hash, info.immutable_samplers[i].ToHash());

        Atomic::CombineHash(hash, info.read_only_depth);

        // Shaders
        Atomic::CombineHash(hash, Atomic::MakeHash(info.vs_shader.ToHash()));
        Atomic::CombineHash(hash, Atomic::MakeHash(info.ps_shader.ToHash()));
        Atomic::CombineHash(hash, Atomic::MakeHash(info.ds_shader.ToHash()));
        Atomic::CombineHash(hash, Atomic::MakeHash(info.hs_shader.ToHash()));
        Atomic::CombineHash(hash, Atomic::MakeHash(info.gs_shader.ToHash()));

        return hash;
    }

    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> pipeline_state_builder_get_or_create_srb(const ShaderResourceBindingCreateDesc& desc)
    {
        u32 key = desc.pipeline_hash;
        // build key from resource pointers
        for (const auto& it : *desc.resources)
            Atomic::CombineHash(key, Atomic::MakeHash(it.second.texture.ConstPtr()));

        if(s_srb.Contains(key))
        {
            auto srb = s_srb[key];
            return srb;
        }

        const auto pipeline = s_pipelines[desc.pipeline_hash];
        if(!pipeline)
        {
            ATOMIC_LOGERRORF("Not found pipeline with given hash #%u", desc.pipeline_hash);
            return {};
        }

        ATOMIC_LOGDEBUGF("Creating SRB with given hash #%u", key);

        Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> srb;
        pipeline->CreateShaderResourceBinding(&srb, true);

        s_srb[key] = srb;


        for(u8 type = 0; type < MAX_SHADER_TYPES; ++type)
        {
	        for(u8 grp = 0; grp < MAX_SHADER_PARAMETER_GROUPS; ++grp)
			{
				const auto shader_type = static_cast<ShaderType>(type);
                const auto d_shader_type = utils_get_shader_type(shader_type);
                const auto group = static_cast<ShaderParameterGroup>(grp);
	            const auto name = utils_get_shader_parameter_group_name(shader_type, group);

				const auto var = srb->GetVariableByName(d_shader_type, name);
                if (var)
                    var->Set(desc.driver->GetConstantBuffer(shader_type, group)->GetGPUObject().Cast<Diligent::IBuffer>(Diligent::IID_Buffer));
			}
		}

        for(const auto& it : *desc.resources)
        {
            for(u8 type = 0; type < MAX_SHADER_TYPES; ++type)
            {
                const auto shader_type = utils_get_shader_type(static_cast<Atomic::ShaderType>(type));
                const auto var = srb->GetVariableByName(shader_type, it.second.name);
                if(var)
                    var->Set(it.second.texture);
            }
        }
        return srb;
    }

    void srb_cache_update_default_cbuffers(const Atomic::ShaderType type, const Atomic::ShaderParameterGroup grp, Diligent::IBuffer* cbuffer)
    {
        using namespace Diligent;
        const auto name = utils_get_shader_parameter_group_name(type, grp);
        const auto shader_type = utils_get_shader_type(type);

        for(const auto& it : s_srb)
        {
	        const auto srb = it.second_;
            const auto var = srb->GetVariableByName(shader_type, name);
            if(var)
                var->Set(cbuffer, SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
        }
    }

    void srb_cache_release()
    {
		s_srb.Clear();
    }

    uint32_t srb_cache_items_count()
    {
	    return s_srb.Size();
    }

}
