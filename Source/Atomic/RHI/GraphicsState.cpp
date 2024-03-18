#include "./GraphicsState.h"

namespace REngine
{
    static GraphicsState s_state = {};
    static RenderCommandState s_default_command = {};

    const GraphicsState& graphics_state_get()
    {
        return s_state;
    }

    void graphics_state_set(const GraphicsState state)
    {
        s_state = state;
    }


    const RenderCommandState& default_render_command_get()
    {
        return s_default_command;
    }

    void default_render_command_set(const RenderCommandState state)
    {
        s_default_command = state;
    }
}
