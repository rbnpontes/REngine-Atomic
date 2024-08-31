#pragma once
#include "./Object.h"
#include "./Plugin.h"
#include "./Variant.h"

namespace Atomic
{
    class ResourceCache;
    class File;
}

namespace REngine {
    using namespace Atomic;

    ENGINE_NO_EXPORT struct PluginEntry
    {
        void* handle_;
        bool loaded_;
        IEnginePlugin* instance_;
    };
    class ATOMIC_API PluginSystem : public Object {
        ATOMIC_OBJECT(REngine::PluginSystem, Object)

    public:
        PluginSystem(Context* context);
        IEnginePlugin* LoadPlugin(const ea::string& plugin_path);
        void UnloadPlugin(u32 plugin_id);
        ea::vector<IEnginePlugin*> GetPlugins() const;
        void Initialize();
    private:
        ResourceCache* GetResourceCache();
        SharedPtr<File> TryOpenLib(ea::string lib_path);
        void HandleEngineExit(StringHash event_type, VariantMap& event_data);
        ea::shared_ptr<PluginEntry> LoadPluginAtPath(const ea::string& plugin_path);
        bool init_;
        ResourceCache* resource_cache_;
        
        ea::unordered_map<u32, ea::shared_ptr<PluginEntry>> plugins_;
    };
}