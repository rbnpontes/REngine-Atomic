#include "Export.h"
#include <Plugins/EngineJS/Interfaces/IJavaScriptSystem.h>

#include "EngineCore/IO/Log.h"

namespace REngine
{
	JavaScriptModulePlugin::JavaScriptModulePlugin() : plugin_id_(0){}

	u32 JavaScriptModulePlugin::GetPluginId() const
	{
		return plugin_id_;
	}

	void JavaScriptModulePlugin::OnPluginLoad(Context* context)
	{
		ATOMIC_LOGDEBUGF("Load %s module", GetModuleName());
		// generate plugin id
		plugin_id_ = StringHash(GetModuleName()).Value();
	}

	void JavaScriptModulePlugin::OnPluginSetup(Context* context)
	{
		
	}
	void JavaScriptModulePlugin::OnPluginStart(Context* context)
	{
		const auto js_plugin = context->GetSubsystem<IJavaScriptSystem>();
		if(!js_plugin)
		{
			ATOMIC_LOGERRORF("Failed to load %s module. IJavaScript system wasn't registered.", GetModuleName());
			return;
		}

		OnSetup(js_plugin->GetHeap());
	}
	void JavaScriptModulePlugin::OnPluginUnload(Context* context)
	{
		ATOMIC_LOGDEBUGF("Unload %s module", GetModuleName());
		const auto js_plugin = context->GetSubsystem<IJavaScriptSystem>();
		if (!js_plugin)
			return;

		OnUnload(js_plugin->GetHeap());
	}
}
