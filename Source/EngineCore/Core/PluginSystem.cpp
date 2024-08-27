#include "./PluginSystem.h"
#include "../Core/StringUtils.h"
#include "../IO/Log.h"
#include "../Resource/ResourceCache.h"
#include "../Engine/EngineEvents.h"

#include <SDL2/SDL.h>

#include "IO/FileSystem.h"

namespace REngine {
    typedef void*(*plugin_entrypoint_fn)();

	PluginSystem::PluginSystem(Context* context) : Object(context),
        init_(false),
        resource_cache_(nullptr)
    {
    }

	ea::vector<IEnginePlugin*> PluginSystem::GetPlugins() const
	{
        ea::vector<IEnginePlugin*> result(plugins_.size());
        u32 i = 0;
        for (const auto& it : plugins_)
            result[++i] = it.second->instance_;
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

        for (const auto& it : plugins_)
            it.second->instance_->OnPluginSetup(context_);
        for (const auto& it : plugins_)
            it.second->instance_->OnPluginStart(context_);

        SubscribeToEvent(E_ENGINE_EXIT, ATOMIC_HANDLER(PluginSystem, HandleEngineExit));
        init_ = true;
    }

    SharedPtr<File> PluginSystem::TryOpenLib(ea::string lib_path)
    {
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

        auto file = resource_cache_->GetFile(lib_path.c_str());
        if (file)
            return file;

        // try to load shared lib from program directory.
        lib_path = ToString("%s/%s", GetSubsystem<FileSystem>()->GetProgramDir().CString(), lib_path.c_str()).CString();

        file = resource_cache_->GetFile(lib_path.c_str());
        if(!file)
            ATOMIC_LOGWARNINGF("Failed to load plugin: %s", lib_path.c_str());

        return file;
    }

    void PluginSystem::HandleEngineExit(StringHash event_type, VariantMap& event_data)
    {
	    for (const auto& it : plugins_)
	    {
            const auto plugin = it.second->instance_;
            plugin->OnPluginUnload(context_);
            ATOMIC_LOGDEBUGF("Unload Plugin %d", plugin->GetPluginId());
	    }

        plugins_.clear(true);
    }

    ea::shared_ptr<PluginEntry> PluginSystem::LoadPluginAtPath(const ea::string& plugin_path)
    {
	    const auto file = TryOpenLib(plugin_path);
        if(!file)
        {
            ATOMIC_LOGWARNINGF("Failed to load plugin: %s", plugin_path.c_str());
            return {};
        }

        ATOMIC_LOGDEBUGF("Loading Plugin: %s", plugin_path.c_str());
        const auto lib = SDL_LoadObject(file->GetFullPath().CString());
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

        const auto plugin_instance = static_cast<IEnginePlugin*>(plugin_entrypoint());
        const auto plugin_it = plugins_.find_as(plugin_instance->GetPluginId());
        if(plugin_it != plugins_.end())
        {
            ATOMIC_LOGERRORF("Plugin %s is already loaded.", plugin_path.c_str());
            SDL_UnloadObject(lib);
            return plugin_it->second;
        }

        plugin_instance->OnPluginLoad(context_);

        const ea::shared_ptr<PluginEntry> entry(new PluginEntry());
        entry->handle_ = lib;
        entry->instance_ = plugin_instance;
        entry->loaded_ = false;
        plugins_[entry->instance_->GetPluginId()] = entry;
        return entry;
    }

    IEnginePlugin* PluginSystem::LoadPlugin(const ea::string& plugin_path)
    {
	    const auto plugin = LoadPluginAtPath(plugin_path);
        if (!plugin)
            return nullptr;

        if (!init_)
            return plugin->instance_;

        if(!plugin->loaded_)
        {
            plugin->instance_->OnPluginSetup(context_);
            plugin->instance_->OnPluginStart(context_);
        }
        return plugin->instance_;
    }

    void PluginSystem::UnloadPlugin(u32 plugin_id)
    {
        const auto& it = plugins_.find_as(plugin_id);
        if(it == plugins_.end())
            return;

        it->second->instance_->OnPluginUnload(context_);
        ATOMIC_LOGDEBUGF("Unload Plugin %d", it->second->instance_->GetPluginId());

		plugins_.erase(it);
    }
}
