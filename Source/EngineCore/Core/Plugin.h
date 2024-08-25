#pragma once
#include "./Context.h"

namespace REngine
{
	using namespace Atomic;
	class IEnginePlugin
	{
	public:
		void OnPluginSetup(Context* context);
		void OnPluginStart(Context* context);
		u32 GetPluginId() const;
	};
}

#define RENGINE_DEFINE_PLUGIN(klass) \
extern "C" ea::shared_ptr<REngine::IEnginePlugin> rengine_plugin_entrypoint(); \
ea::shared_ptr<REngine::IEnginePlugin> rengine_plugin_entrypoint() \
{ \
	ea::shared_ptr<REngine::IEnginePlugin> plugin(new klass()); \
	return plugin; \
}
