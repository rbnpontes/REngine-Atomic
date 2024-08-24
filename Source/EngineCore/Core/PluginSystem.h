#pragma once
#include "./Object.h"

namespace REngine {
    using namespace Atomic;
    class ATOMIC_API PluginSystem : Object {
        ATOMIC_OBJECT(PluginSystem, Object);
        public:
            PluginSystem(Context* context);
    };
}