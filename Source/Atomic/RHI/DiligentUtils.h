#pragma once
#include <DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h>

#include "Graphics/GraphicsDefs.h"

namespace REngine
{
    using namespace Diligent;
    using namespace Atomic;
    void get_primitive_type(unsigned element_count,
        PrimitiveType type,
        unsigned& primitive_count,
        PRIMITIVE_TOPOLOGY primitive_topology);
}
