#pragma once
#include <DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h>

#include "Graphics/GraphicsDefs.h"

namespace REngine
{
    using namespace Diligent;
    using namespace Atomic;
    void utils_get_primitive_type(unsigned element_count,
        PrimitiveType type,
        unsigned& primitive_count,
        PRIMITIVE_TOPOLOGY primitive_topology);

    unsigned utils_calc_sub_resource(unsigned mip_slice, unsigned array_slice, unsigned mip_levels);

    const char* utils_get_shader_parameter_group_name(Atomic::ShaderType type, Atomic::ShaderParameterGroup grp);
    Atomic::ShaderParameterGroup utils_get_shader_parameter_group_type(const Atomic::String& name);
    Atomic::VertexElementSemantic utils_get_element_semantic(const Atomic::String& name, uint8_t* index);
    Atomic::TextureUnit utils_get_texture_unit(const Atomic::String& name);
}
