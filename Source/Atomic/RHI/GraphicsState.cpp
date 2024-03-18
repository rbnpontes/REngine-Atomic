#include "./GraphicsState.h"

namespace REngine
{
    static GraphicsState s_state = {};

    const GraphicsState& graphics_state_get()
    {
        return s_state;
    }

    void graphics_state_set(const GraphicsState state)
    {
        s_state = state;
    }

}
