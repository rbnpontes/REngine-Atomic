#pragma once
#include "./Object.h"
#include "./Plugin.h"
#include "./Variant.h"

namespace Atomic
{
    class ResourceCache;
}
namespace REngine {
    using namespace Atomic;

    ENGINE_NO_EXPORT struct PluginEntry
    {
        void* handle;
        ea::shared_ptr<REngine::IEnginePlugin> instance;
    };
    class ATOMIC_API PluginSystem : public Object {
        ATOMIC_OBJECT(PluginSystem, Object)

    public:
        PluginSystem(Context* context);
        void LoadPlugin(const ea::string& plugin_path);
        void UnloadPlugin(u32 plugin_id);
        ea::vector<ea::shared_ptr<IEnginePlugin>> GetPlugins() const;
        void Initialize();
    private:
        void HandleEngineExit(StringHash event_type, VariantMap& event_data);
        ea::shared_ptr<REngine::IEnginePlugin> LoadPluginAtPath(const ea::string& plugin_path);
        bool init_;
        ResourceCache* resource_cache_;

        ea::stack<ea::string> plugins_2_load_;
        ea::unordered_map<u32, ea::shared_ptr<PluginEntry>> plugins_;
    };
}