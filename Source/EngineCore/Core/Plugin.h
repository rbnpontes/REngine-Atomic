#pragma once
#include "./Context.h"

namespace REngine
{
	using namespace Atomic;
	class IEnginePlugin
	{
	public:
		virtual void OnPluginLoad(Context* context) = 0;
		virtual void OnPluginUnload(Context* context) = 0;
		virtual void OnPluginSetup(Context* context) = 0;
		virtual void OnPluginStart(Context* context) = 0;
		virtual u32 GetPluginId() const = 0;
	};
}

#define ENGINE_DEFINE_PLUGIN(klass) \
ENGINE_DEFINE_PLUGIN_SIGNATURE \
void* rengine_plugin_entrypoint() \
{ \
	static REngine::IEnginePlugin* plugin = new klass(); \
	return plugin; \
}
