#pragma once
#include "./Object.h"
#include "./Plugin.h"

namespace REngine {
    using namespace Atomic;
    class ATOMIC_API PluginSystem : public Object {
        ATOMIC_OBJECT(PluginSystem, Object)

    public:
        PluginSystem(Context* context);
        void LoadPlugin(const ea::string& plugin_path);
        void Initialize();
    private:
        bool init_;

        ea::stack<ea::string> plugins_2_load_;
        ea::unordered_map<u32, ea::shared_ptr<REngine::IEnginePlugin>> plugins_;
    };
}