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

    Atomic::SharedPtr<Atomic::ConstantBuffer> graphics_state_get_constant_buffer(unsigned id)
    {
        const auto it = s_state.all_constant_buffers.Find(id);
        if(it == s_state.all_constant_buffers.End())
            return {};
        return it->second_;
    }

    void graphics_state_add_constant_buffer(const unsigned index, Atomic::SharedPtr<Atomic::ConstantBuffer>& buffer)
    {
        s_state.all_constant_buffers[index] = buffer;
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
