#include "DiligentUtils.h"
#include "../Core/StringUtils.h"
namespace REngine
{
    static const char* s_shader_param_grp_names[MAX_SHADER_TYPES * MAX_SHADER_PARAMETER_GROUPS] = {
        "FrameVS",
        "CameraVS",
        "ZoneVS",
        "LightVS",
        "MaterialVS",
        "ObjectVS",
        "CustomVS",
        "FramePS",
        "CameraPS",
        "ZonePS",
        "LightPS",
        "MaterialPS",
        "ObjectPS",
        "CustomPS",
    };
    static const char* s_semantic_names[] = {
        "POS",
        "NORMAL",
        "BINORMAL",
        "TANGENT",
        "TEXCOORD",
        "COLOR",
        "BLENDWEIGHT",
        "BLENDINDICES",
        "OBJECTINDEX"
    };
    static Atomic::HashMap<StringHash, TextureUnit> s_texture_unit_map = {
        { "sDiffMap", TU_DIFFUSE },
        {"sDiffCubeMap", TU_DIFFUSE},
        {"sNormalMap", TU_NORMAL},
        {"sSpecMap", TU_SPECULAR},
        {"sEmissiveMap", TU_EMISSIVE},
        {"sEnvMap", TU_ENVIRONMENT},
        {"sEnvCubeMap", TU_ENVIRONMENT},
        {"sLightRampMap", TU_LIGHTRAMP},
        {"sLightSpotMap", TU_LIGHTSHAPE},
        {"sLightCubeMap", TU_LIGHTSHAPE},
        {"sShadowMap", TU_SHADOWMAP},
        {"sFaceSelectCubeMap", TU_FACESELECT},
        {"sIndirectionCubeMap", TU_INDIRECTION},
        {"sVolumeMap", TU_VOLUMEMAP},
        {"sZoneCubeMap", TU_ZONE},
        {"sZoneVolumeMap",TU_ZONE}
    };

    static Atomic::StringVector s_texture_unit_names[] = {
        {"sDiffMap", "sDiffCubeMap"},    // TU_DIFFUSE
        {"sNormalMap"},                    // TU_NORMAL
        {"sSpecMap"},                      // TU_SPECULAR
        {"sEmissiveMap"},                  // TU_EMISSIVE
        {"sEnvMap", "sEnvCubeMap"},     // TU_ENVIRONMENT
#ifdef DESKTOP_GRAPHICS
        {"sVolumeMap"},                    // TU_VOLUMEMAP
        {"sCustom", "sCustom1"},        // TU_CUSTOM1
        {"sCustom", "sCustom1"},        // TU_CUSTOM2
        {"sLightRampMap"},                 // TU_LIGHTRAMP
        {"sLightSpotMap", "sLightCubeMap"}, // TU_LIGHTSHAPE
        {"sShadowMap"},                     // TU_SHADOWMAP
        {"sFaceSelectCubeMap"},             // TU_FACESELECT
        {"sIndirectionCubeMap"},            // TU_INDIRECTION
        { "sDepthBuffer"},                  // TU_DEPTHBUFFER
        {"sLightBuffer"},   	              // TU_LIGHTBUFFER
        {"sZoneCubeMap", "sZoneVolumeMap"} ,// TU_ZONE
        {"sAlbedoBuffer"}, // TU_ALBEDOBUFFER
        {"sNormalBuffer"} // TU_NORMALBUFFER
#else
        {"sLightRampMap"} // TU_LIGHTRAMP
        {"sLightSpotMap", "sLightCubeMap"}, // TU_LIGHTSHAPE
        {"sShadowMap"},                     // TU_SHADOWMAP
#endif
    };


    static Diligent::SHADER_TYPE s_shader_types[] = {
    		Diligent::SHADER_TYPE_VERTEX,
    		Diligent::SHADER_TYPE_PIXEL,
    		Diligent::SHADER_TYPE_GEOMETRY,
    		Diligent::SHADER_TYPE_HULL,
    		Diligent::SHADER_TYPE_DOMAIN,
    		Diligent::SHADER_TYPE_COMPUTE
    	};


    static unsigned NumberPostfix(const Atomic::String& str)
    {
        for (unsigned i = 0; i < str.Length(); ++i)
        {
            if (Atomic::IsDigit(str[i]))
                return Atomic::ToUInt(str.CString() + i);
        }

        return M_MAX_UNSIGNED;
    }

    Diligent::PRIMITIVE_TOPOLOGY utils_get_primitive_type(unsigned element_count, PrimitiveType type, uint32_t* primitive_count)
    {
        using namespace Diligent;
        switch (type)
        {
        case TRIANGLE_LIST:
            *primitive_count = element_count / 3;
            return PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case LINE_LIST:
            *primitive_count = element_count / 2;
            return PRIMITIVE_TOPOLOGY_LINE_LIST;
        case POINT_LIST:
            *primitive_count = element_count;
            return PRIMITIVE_TOPOLOGY_POINT_LIST;
        case TRIANGLE_STRIP:
            *primitive_count = element_count - 2;
            return PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case LINE_STRIP:
            *primitive_count = element_count - 1;
            return PRIMITIVE_TOPOLOGY_LINE_STRIP;
        default:
            *primitive_count = 0;
            return PRIMITIVE_TOPOLOGY_UNDEFINED;
        }
    }

    unsigned utils_calc_sub_resource(unsigned mip_slice, unsigned array_slice, unsigned mip_levels)
    {
        return mip_slice + (array_slice * mip_levels);
    }

    const char* utils_get_shader_parameter_group_name(Atomic::ShaderType type, Atomic::ShaderParameterGroup grp)
    {
        const auto idx = type * static_cast<uint32_t>(MAX_SHADER_PARAMETER_GROUPS) + grp;
        assert(idx < _countof(s_shader_param_grp_names), "Invalid Shader Type or Shader Parameter Group");
        return s_shader_param_grp_names[idx];    
    }

    Atomic::ShaderParameterGroup utils_get_shader_parameter_group_type(Atomic::ShaderType type, const Atomic::String& name)
    {
        for(uint8_t i =0; i < Atomic::MAX_SHADER_PARAMETER_GROUPS; ++i)
        {
	        const auto idx = static_cast<uint32_t>(type) * static_cast<uint32_t>(MAX_SHADER_PARAMETER_GROUPS) + i;
            Atomic::String target = s_shader_param_grp_names[idx];
            if (target.StartsWith(name))
                return static_cast<Atomic::ShaderParameterGroup>(i);
        }
        return Atomic::MAX_SHADER_PARAMETER_GROUPS;
    }

    Atomic::VertexElementSemantic utils_get_element_semantic(const Atomic::String& name, uint8_t* index)
    {
        *index = 0;
        for(uint32_t i = MAX_VERTEX_ELEMENT_SEMANTICS - 1; i <MAX_VERTEX_ELEMENT_SEMANTICS; --i)
        {
	        if(!name.Contains(s_semantic_names[i], false))
                continue;;
	        const auto semantic_index = NumberPostfix(name);
            if (semantic_index != M_MAX_UNSIGNED)
                *index = static_cast<uint8_t>(semantic_index);
            return static_cast<Atomic::VertexElementSemantic>(i);
        }
        return MAX_VERTEX_ELEMENT_SEMANTICS;
    }

    Atomic::TextureUnit utils_get_texture_unit(const Atomic::String& name)
    {
        const auto& it = s_texture_unit_map.Find(name);
        return it != s_texture_unit_map.End() ? it->second_ : MAX_TEXTURE_UNITS;
    }

    Atomic::StringVector utils_get_texture_unit_names(Atomic::TextureUnit unit)
    {
        if (unit >= MAX_TEXTURE_UNITS)
            return {};

        const auto names = s_texture_unit_names[unit];
        return names;
    }

    Diligent::SHADER_TYPE utils_get_shader_type(Atomic::ShaderType type)
    {
        return s_shader_types[type];
    }

    bool utils_is_compressed_texture_format(Diligent::TEXTURE_FORMAT format)
    {
        using namespace Diligent;
        return format >= TEX_FORMAT_BC1_TYPELESS && format <= TEX_FORMAT_BC5_SNORM;
    }

    Diligent::ITextureView* utils_create_texture_view(Diligent::ITexture* texture,
                                                      Diligent::TEXTURE_VIEW_TYPE view_type)
    {
        Diligent::ITextureView* view = texture->GetDefaultView(view_type);
        if(view)
        {
	        texture->AddRef();
			return view;
        }

    	Diligent::TextureViewDesc view_desc = {};
        view_desc.Name = texture->GetDesc().Name;
		view_desc.ViewType = view_type;
		view_desc.Format = texture->GetDesc().Format;
		view_desc.TextureDim = texture->GetDesc().Type;

        if(texture->GetDesc().MiscFlags & Diligent::MISC_TEXTURE_FLAG_GENERATE_MIPS)
            view_desc.Flags |= Diligent::TEXTURE_VIEW_FLAG_ALLOW_MIP_MAP_GENERATION;

		texture->CreateView(view_desc, &view);
        return view;
    }
}
