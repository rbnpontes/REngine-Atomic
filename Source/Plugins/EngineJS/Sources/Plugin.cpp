#include <Plugins/EngineJS/Interfaces/IJavaScriptPlugin.h>
#include "./JavaScriptSystem.h"

namespace REngine
{
	static StringHash s_js_plugin_id = "{E4864575-BF10-4D13-B681-F22FFD84DD77}";
	class JavaScriptPluginImpl : public IJavaScriptPlugin
	{
	public:
		JavaScriptPluginImpl() : js_system_(nullptr)
		{
		}

		void OnPluginLoad(Context* context) override
		{
			const auto js_sys = new JavaScriptSystem(context);
			context->RegisterSubsystem<IJavaScriptSystem>(js_sys);

			js_sys->Initialize();

			js_system_ = js_sys;
		}

		void OnPluginUnload(Context* context) override
		{
			context->RemoveSubsystem<IJavaScriptSystem>();
			js_system_ = nullptr;
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
			return js_system_;
		}
	private:
		IJavaScriptSystem* js_system_;
	};

	ENGINE_DEFINE_PLUGIN(JavaScriptPluginImpl);
}