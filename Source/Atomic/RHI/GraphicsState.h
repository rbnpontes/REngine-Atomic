#pragma once
#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>

#include "./RenderCommand.h"

namespace REngine
{
    using namespace Diligent;
    struct GraphicsState
    {
        ISwapChain* current_swap_chain;
    };
    
    const GraphicsState& graphics_state_get();
    void graphics_state_set(const GraphicsState state);

    const RenderCommandState& default_render_command_get();
    void default_render_command_set(const RenderCommandState state);

    
}
