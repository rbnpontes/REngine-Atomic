#pragma once
#include "./RenderCommand.h"
#include "../Container/HashMap.h"
#include "../Graphics/ConstantBuffer.h"
#include "../Container/Ptr.h"

#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>

namespace REngine
{
    using namespace Diligent;
    struct GraphicsState
    {
        Atomic::HashMap<unsigned, Atomic::SharedPtr<Atomic::ConstantBuffer>> all_constant_buffers;
    };
    
    const GraphicsState& graphics_state_get();
    void graphics_state_set(const GraphicsState state);
    Atomic::SharedPtr<Atomic::ConstantBuffer> graphics_state_get_constant_buffer(unsigned id);
    void graphics_state_add_constant_buffer(const unsigned index, Atomic::SharedPtr<Atomic::ConstantBuffer>& buffer);
    
    const RenderCommandState& default_render_command_get();
    void default_render_command_set(const RenderCommandState state);
}
