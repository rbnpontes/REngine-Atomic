#include "DiligentUtils.h"

namespace REngine
{
    void utils_get_primitive_type(unsigned element_count, PrimitiveType type, unsigned& primitive_count, PRIMITIVE_TOPOLOGY primitive_topology)
    {
        switch (type)
        {
        case TRIANGLE_LIST:
            primitive_count = element_count / 3;
            primitive_topology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        case LINE_LIST:
            primitive_count = element_count / 2;
            primitive_topology = PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        case POINT_LIST:
            primitive_count = element_count;
            primitive_topology = PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case TRIANGLE_STRIP:
            primitive_count = element_count - 2;
            primitive_topology = PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            break;
        case LINE_STRIP:
            primitive_count = element_count - 1;
            primitive_topology = PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break;
        default:
            primitive_count = 0;
            primitive_topology = PRIMITIVE_TOPOLOGY_UNDEFINED;
            break;
        }
    }

    unsigned utils_calc_sub_resource(unsigned mip_slice, unsigned array_slice, unsigned mip_levels)
    {
        return mip_slice + (array_slice * mip_levels);
    }

}
