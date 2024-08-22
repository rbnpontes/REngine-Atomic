#pragma once
#include "../Container/TypeTraits.h"

class IEnginePlugin {
    public:
        virtual void OnPluginLoad() = 0;
        virtual void OnPluginUnload() = 0;
        virtual void OnPluginStart() = 0;
        virtual void OnPluginSetup() = 0;
        virtual void OnPluginStop() = 0;
        virtual u32 GetPluginId() = 0;
}