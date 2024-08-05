//
// Copyright (c) 2014-2016 THUNDERBEAST GAMES LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <EngineCore/Input/Input.h>
#include <EngineCore/UI/UI.h>
#include <EngineCoreJS/Javascript/Javascript.h>
#include <EngineCore/IPC/IPC.h>
#include <EngineCore/Engine/EngineDefs.h>

// This can be removed once bone hack is fixed
#include <EngineCore/Graphics/AnimatedModel.h>

#include <ToolCore/License/LicenseSystem.h>
#include <ToolCore/ToolSystem.h>
#include <ToolCore/ToolEnvironment.h>
#include <ToolCore/NETTools/NETBuildSystem.h>

#include "../EditorMode/AEEditorMode.h"
#include "../EditorMode/AEEditorNETService.h"

#include "../Components/EditorComponents.h"

#include "AEEditorPrefs.h"
#include "AEEditorApp.h"

#include <EngineCore/Web/Web.h>

using namespace ToolCore;

// Fix these externs
namespace Atomic
{
#ifdef ENGINE_WEBVIEW
    void jsapi_init_webview(JSVM* vm, const VariantMap& engineParameters);
#endif
#ifdef ENGINE_DOTNET
    extern void jsb_package_enginenetscript_init(JSVM* vm);
#endif
}

namespace ToolCore
{
    extern void jsapi_init_toolcore(JSVM* vm);
}

using namespace ToolCore;

namespace AtomicEditor
{

    extern void jsapi_init_editor(JSVM* vm);

    void rengine_declare_features(duk_context* ctx)
    {
        bool dotnet_enabled = false;
        bool webview_enabled = false;
#ifdef ENGINE_DOTNET
        dotnet_enabled = true;
#endif
#ifdef ENGINE_WEBVIEW
        webview_enabled = true;
#endif

        duk_push_object(ctx);
        // features.dotnet
    	duk_push_boolean(ctx, dotnet_enabled);
        duk_put_prop_string(ctx, -2, "dotnet");
        // features.webview
    	duk_push_boolean(ctx, webview_enabled);
        duk_put_prop_string(ctx, -2, "webview");

        duk_put_global_string(ctx, "features");
    }

    AEEditorApp::AEEditorApp(Context* context) :
        AppBase(context)
    {

    }

    AEEditorApp::~AEEditorApp()
    {

    }

    void AEEditorApp::Setup()
    {
        context_->RegisterSubsystem(new AEEditorPrefs(context_));

        context_->SetEditorContext(true);

        AppBase::Setup();

        RegisterEditorComponentLibrary(context_);

        // Register IPC system
        context_->RegisterSubsystem(new IPC(context_));

        ToolEnvironment* env = new ToolEnvironment(context_);
        context_->RegisterSubsystem(env);

        env->Initialize();

        ToolSystem* system = new ToolSystem(context_);
        context_->RegisterSubsystem(system);

        engineParameters_["WindowTitle"] = ENGINE_EDITOR_NAME;
        engineParameters_["WindowResizable"] = true;
        engineParameters_["FullScreen"] = false;
        engineParameters_["LogLevel"] = LOG_DEBUG;

        FileSystem* filesystem = GetSubsystem<FileSystem>();
        engineParameters_["LogName"] = filesystem->GetAppPreferencesDir(ENGINE_EDITOR_NAME, "Logs") + String(ENGINE_EDITOR_NAME) + ".log";

#ifdef ENGINE_PLATFORM_MACOS
        engineParameters_["WindowIcon"] = "Images/EngineLogo32.jpg";
#endif

        String resource_prefix_paths;
        String resource_paths;
#ifdef ENGINE_DEV_BUILD
        resource_paths = env->GetCoreDataDir() + ";" + env->GetEditorDataDir();
        // for dev builds, add the compile editor scripts from artifacts
        resource_paths += ";" + env->GetRootSourceDir() + "Artifacts/Build/Resources/EditorData/;";
#else
        resource_paths += "CoreData;EditorData";

    #ifdef ENGINE_PLATFORM_MACOS
        resource_prefix_paths = filesystem->GetProgramDir() + "../Resources";
    #else
        resource_prefix_paths = filesystem->GetProgramDir() + "Resources";
    #endif
#endif // ENGINE_DEV_BUILD

        engineParameters_["ResourcePaths"] = resource_paths;
        engineParameters_["ResourcePrefixPaths"] = resource_prefix_paths;

        ATOMIC_LOGINFOF("ResourcePaths: %s", resource_paths.CString());
        ATOMIC_LOGINFOF("ResourcePrefixPaths: %s", resource_prefix_paths.CString());

        engineParameters_[EP_PROFILER_LISTEN] = false;

        GetSubsystem<AEEditorPrefs>()->ReadPreferences(engineParameters_);

        // Register JS packages

        JSVM::RegisterPackage(jsapi_init_toolcore);
        JSVM::RegisterPackage(jsapi_init_editor);
#ifdef ENGINE_DOTNET
        JSVM::RegisterPackage(jsb_package_enginenetscript_init);
#endif
#ifdef ENGINE_WEBVIEW
        JSVM::RegisterPackage(jsapi_init_webview, engineParameters_);
#endif

    }

    void AEEditorApp::Start()
    {
        GetSubsystem<AEEditorPrefs>()->ValidateWindow();

        context_->RegisterSubsystem(new EditorMode(context_));
#ifdef ENGINE_DOTNET
        context_->RegisterSubsystem(new NETBuildSystem(context_));
#endif
        context_->RegisterSubsystem(new EditorNETService(context_));        

        AppBase::Start();

        vm_->SetModuleSearchPaths("JavaScript;EditorScripts;EngineEditor");

        // move UI initialization to JS
        UI* ui = GetSubsystem<UI>();
        ui->Initialize("resources/language/lng_en.tb.txt");        

        rengine_declare_features(vm_->GetJSContext());
        duk_get_global_string(vm_->GetJSContext(), "require");
        duk_push_string(vm_->GetJSContext(), "main");
        if (duk_pcall(vm_->GetJSContext(), 1) != 0)
        {
            vm_->SendJSErrorEvent();
            ErrorExit("Error executing main.js");
        }

        // uncomment this to enable license modal
        //GetSubsystem<LicenseSystem>()->Initialize();

        Input* input = GetSubsystem<Input>();
        // Ensure exclusive fullscreen is disabled in Editor application
        input->SetToggleFullscreen(false);
        input->SetMouseVisible(true);

    }

    void AEEditorApp::Stop()
    {
        IPC* ipc = GetSubsystem<IPC>();

        if (ipc)
        {
            ipc->Shutdown();
        }

        AppBase::Stop();
    }

    void AEEditorApp::ProcessArguments()
    {
        AppBase::ProcessArguments();
    }

}
