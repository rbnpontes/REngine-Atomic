#pragma once
#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>

namespace REngine
{
    using namespace Diligent;
    struct GraphicsState
    {
        ISwapChain* current_swap_chain;
    };

    const GraphicsState& graphics_state_get();
    void graphics_state_set(const GraphicsState state);
}
