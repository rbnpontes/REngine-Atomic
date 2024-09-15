#include <EngineCore/Core/ProcessUtils.h>
#include <EngineCore/Core/PluginSystem.h>
#include <EngineCore/IO/FileSystem.h>
#include <EngineCore/IO/Log.h>
#include <EngineCore/Resource/ResourceCache.h>
#include <EngineCore/Scene/Scene.h>
#include <EngineCore/Resource/Localization.h>
#include <EngineCore/Engine/EngineEvents.h>
#include <Plugins/EngineJS/Interfaces/IJavaScriptPlugin.h>
#include <csignal>
#include <iostream>

using namespace Atomic;
using namespace REngine;

static bool g_stop = false;
void on_signal_handler(i32 signal)
{
	if (signal != SIGINT)
		return;

	ATOMIC_LOGDEBUG("Exiting JavaScript Sandbox!");
	g_stop = true;
}

void register_js_methods(duk_context* ctx)
{
	// exit call
	duk_push_c_function(ctx, [](duk_context* _)
	{
		g_stop = true;
		return 0;
	}, 0);
	duk_put_global_string(ctx, "exit");
}

int main(int argc, char** argv)
{
	const auto arguments = ParseArguments(argc, argv);
	const auto context = new Context();
	const auto plugin_system = new PluginSystem(context);
	const auto log = new Log(context);
	const auto resource_cache = new ResourceCache(context);

	// Initialize some subsystems
	context->RegisterSubsystem(plugin_system);
	context->RegisterSubsystem(new FileSystem(context));
	context->RegisterSubsystem(log);
	context->RegisterSubsystem(resource_cache);
	context->RegisterSubsystem(new Localization(context));

	RegisterSceneLibrary(context);
	context->InitSubsystemCache();

	context->GetResourceCache()->AddResourceDir(context->GetFileSystem()->GetProgramDir());

	// Init log
	log->SetQuiet(false);
	log->Open("JavaScriptSandbox.log");

	// Loading JavaScript Plugin
	const auto js_plugin = static_cast<IJavaScriptPlugin*>(plugin_system->LoadPlugin(ENGINE_PLUGIN_JS_TARGET));
	plugin_system->Initialize();

	// Register Global JS methods
	register_js_methods(js_plugin->GetJavaScriptSystem(context)->GetHeap());

	signal(SIGINT, on_signal_handler);

	const auto js_system = js_plugin->GetJavaScriptSystem(context);
	// If argument is set, probably is a js file
	ea::string js_file_path;
	if(arguments.Size() > 0)
	{
		for(const auto& arg : arguments)
		{
			if(arg.EndsWith(".js"))
			{
				js_file_path = arg.ToStdString();
				break;
			}
		}

		if(js_file_path.empty())
			ATOMIC_LOGERROR("Not found a javascript file.");
		else
		{
			js_system->EvalFromFilePath(js_file_path);
			// execute timers until there's no timer alive.
			while (js_system->HasPendingTimers())
				js_system->ExecutePendingTimers();
		}
	}
	else
	{
		std::cout << ":: [JavaScript Sandbox Tool] ::\n";
		std::cout << "Type 'exit()' to exit sandbox or CTRL + C\n";

		std::string code;
		while (!g_stop)
		{
			std::cout << "> ";
			std::getline(std::cin, code);

			if(code.empty())
				continue;

			js_system->Eval(code.c_str());
			js_system->ExecutePendingTimers();
		}
	}

	VariantMap event_data;
	plugin_system->SendEvent(E_ENGINE_EXIT, event_data);

	return 0;
}
