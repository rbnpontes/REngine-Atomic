#pragma once
#include <EngineCore/Core/Plugin.h>
#include <Duktape/duktape.h>

namespace REngine
{
	class JavaScriptModulePlugin : public IEnginePlugin
	{
	public:
		JavaScriptModulePlugin();
		void OnPluginLoad(Context* context) override;
		void OnPluginUnload(Context* context) override;
		void OnPluginSetup(Context* context) override;
		void OnPluginStart(Context* context) override;
		u32 GetPluginId() const override;
	protected:
		virtual const char* GetModuleName() = 0;
		virtual void OnSetup(duk_context* ctx) = 0;
		virtual void OnUnload(duk_context* ctx) = 0;

		u32 plugin_id_;
	};
}


#define export_module(module_name, setup_fn, unload_fn) \
	class module_name##Plugin : public JavaScriptModulePlugin \
	{ \
	protected: \
		const char* GetModuleName() override { return #module_name; } \
		void OnSetup(duk_context* ctx) override { setup_fn##(ctx); } \
		void OnUnload(duk_context* ctx) override { unload_fn##(ctx); } \
	}; \
	ENGINE_DEFINE_PLUGIN(module_name##Plugin)
