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
}
