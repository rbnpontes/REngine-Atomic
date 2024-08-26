#include "./PluginSystem.h"
#include "../Core/StringUtils.h"
#include "../IO/Log.h"
#include "../Resource/ResourceCache.h"
#include "../Engine/EngineEvents.h"

#include <SDL2/SDL.h>

namespace REngine {
    typedef ea::shared_ptr<IEnginePlugin>(*plugin_entrypoint_fn)();

	PluginSystem::PluginSystem(Context* context) : Object(context),
        init_(false),
        resource_cache_(nullptr)
    {
    }

	ea::vector<ea::shared_ptr<IEnginePlugin>> PluginSystem::GetPlugins() const
	{
        ea::vector<ea::shared_ptr<IEnginePlugin>> result(plugins_.size());
        u32 i = 0;
        for (const auto& it : plugins_)
            result[++i] = it.second->instance;
        return result;
	}

    void PluginSystem::Initialize()
    {
	    if(init_)
	    {
            ATOMIC_LOGWARNING("PluginSystem is already initialized.");
            return;
	    }

        resource_cache_ = GetSubsystem<ResourceCache>();

        ea::vector<ea::shared_ptr<REngine::IEnginePlugin>> created_plugins;
        while(!plugins_2_load_.empty())
        {
            const auto plugin_path = plugins_2_load_.top();
            plugins_2_load_.pop();

            const auto plugin = LoadPluginAtPath(plugin_path);
            created_plugins.push_back(plugin);
        }

        for (const auto& plugin : created_plugins)
            plugin->OnPluginSetup(context_);
        for (const auto& plugin : created_plugins)
            plugin->OnPluginStart(context_);

        SubscribeToEvent(E_ENGINE_EXIT, ATOMIC_HANDLER(PluginSystem, HandleEngineExit));
        init_ = true;
    }

    void PluginSystem::HandleEngineExit(StringHash event_type, VariantMap& event_data)
    {
	    for (const auto& it : plugins_)
	    {
            const auto plugin = it.second->instance;
            plugin->OnPluginUnload(context_);
            ATOMIC_LOGDEBUGF("Unload Plugin %d", plugin->GetPluginId());
	    }

        plugins_.clear(true);
    }

    ea::shared_ptr<REngine::IEnginePlugin> PluginSystem::LoadPluginAtPath(const ea::string& plugin_path)
    {
        ea::string lib_path = plugin_path;
#ifdef ENGINE_PLATFORM_WINDOWS
        if (!string_ends_with(lib_path, ".dll"))
            lib_path += ".dll";
#elif ENGINE_PLATFORM_LINUX|| ENGINE_PLATFORM_ANDROID
        if (!string_ends_with(lib_path, ".so"))
            lib_path += ".so";
#elif ENGINE_PLATFORM_MACOS || ENGINE_PLATFORM_IOS
        if (!string_ends_with(lib_path, ".dylib"))
            lib_path += ".dylib";
#endif

        auto file = resource_cache_->GetFile(plugin_path.c_str());
        if(!file)
        {
            ATOMIC_LOGWARNINGF("Failed to load plugin: %s", plugin_path.c_str());
            return {};
        }

		lib_path = file->GetFullPath().CString();

        ATOMIC_LOGDEBUGF("Loading Plugin: %s", plugin_path.c_str());
        auto lib = SDL_LoadObject(lib_path.c_str());
        if(!lib)
        {
            ATOMIC_LOGERRORF("Failed to load plugin: %s", plugin_path.c_str());
            return {};
        }


        const auto plugin_entrypoint = static_cast<plugin_entrypoint_fn>(SDL_LoadFunction(lib, "rengine_plugin_entrypoint"));
        if(!plugin_entrypoint)
        {
            ATOMIC_LOGERRORF("Invalid Plugin %s, Plugin doesn't contains expected entrypoint.", plugin_path.c_str());
            SDL_UnloadObject(lib);
            return {};
        }

        const auto plugin_instance = plugin_entrypoint();
        if(plugins_.find_as(plugin_instance->GetPluginId()) == plugins_.end())
        {
            ATOMIC_LOGERRORF("Plugin %s is already loaded.", plugin_path.c_str());
            SDL_UnloadObject(lib);
            return {};
        }

        plugin_instance->OnPluginLoad(context_);

        const ea::shared_ptr<PluginEntry> entry(new PluginEntry());
        entry->handle = lib;
        entry->instance = plugin_entrypoint();
        plugins_[entry->instance->GetPluginId()] = entry;
        return entry->instance;
    }

    void PluginSystem::LoadPlugin(const ea::string& plugin_path)
    {
        if (init_)
        {
	        const auto plugin = LoadPluginAtPath(plugin_path);
            plugin->OnPluginSetup(context_);
            plugin->OnPluginStart(context_);
            return;
        }

        plugins_2_load_.emplace_back(plugin_path);
    }

    void PluginSystem::UnloadPlugin(u32 plugin_id)
    {
        const auto& it = plugins_.find_as(plugin_id);
        if(it == plugins_.end())
            return;

        it->second->instance->OnPluginUnload(context_);
        ATOMIC_LOGDEBUGF("Unload Plugin %d", it->second->instance->GetPluginId());

		plugins_.erase(it);
    }
}
