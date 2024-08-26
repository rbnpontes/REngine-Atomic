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

#define RENGINE_DEFINE_PLUGIN(klass) \
extern "C" ea::shared_ptr<REngine::IEnginePlugin> rengine_plugin_entrypoint(); \
ea::shared_ptr<REngine::IEnginePlugin> rengine_plugin_entrypoint() \
{ \
	ea::shared_ptr<REngine::IEnginePlugin> plugin(new klass()); \
	return plugin; \
}
