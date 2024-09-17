#include <Plugins/EngineJS/Interfaces/IJavaScriptPlugin.h>
#include "./JavaScriptSystem.h"

namespace REngine
{
	static StringHash s_js_plugin_id = "{E4864575-BF10-4D13-B681-F22FFD84DD77}";
	class JavaScriptPluginImpl : public IJavaScriptPlugin
	{
	public:
		void OnPluginLoad(Context* context) override
		{
			const auto js_sys = new JavaScriptSystem(context);
			context->RegisterSubsystem(js_sys);

			js_sys->Initialize();
		}

		void OnPluginUnload(Context* context) override
		{
			context->RemoveSubsystem<JavaScriptSystem>();
		}

		void OnPluginSetup(Context* context) override
		{
			
		}

		void OnPluginStart(Context* context) override
		{
			
		}

		u32 GetPluginId() const override
		{
			return s_js_plugin_id.Value();
		}

		IJavaScriptSystem* GetJavaScriptSystem(Context* context) override
		{
			return context->GetSubsystem<JavaScriptSystem>();
		}
	};

	ENGINE_DEFINE_PLUGIN(JavaScriptPluginImpl);
}