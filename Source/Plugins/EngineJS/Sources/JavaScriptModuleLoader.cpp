#include "JavaScriptModuleLoader.h"
#include <EngineCore/Core/StringUtils.h>

#include <EngineCore/IO/FileSystem.h>
#include <EngineCore/Resource/ResourceCache.h>

#include <filesystem>

#include "JavaScriptSystem.h"
#include "JavaScriptAssert.h"

#define JS_REQUIRE_PROP "require"
#define JS_CACHE_PROP "cache"

namespace REngine
{
	ea::string js_module_loader__get_script_path(duk_context* ctx, const ea::string& base_path, const ea::string& module_path)
	{
		std::filesystem::path a = base_path.c_str();
		std::filesystem::path b = module_path.c_str();

		std::filesystem::path combined_path = a / b;
		combined_path = std::filesystem::absolute(combined_path).lexically_normal();

		return ea::string(combined_path.string().c_str());
		
	}

	Atomic::SharedPtr<Atomic::File> js_module_loader__load_file_module(duk_context* ctx, const Atomic::Context* engine_ctx, ea::string module_path)
	{
		const auto file_sys = engine_ctx->GetFileSystem();

		duk_get_global_string(ctx, "__dirname");
		if (duk_is_null_or_undefined(ctx, -1))
			return nullptr;

		ea::string dir_name = duk_get_string(ctx, -1);
		if (dir_name.empty())
			dir_name = file_sys->GetProgramDir().CString();
		duk_pop(ctx);

		module_path = js_module_loader__get_script_path(ctx, dir_name, module_path);

		char delimiter = '/';
#if ENGINE_PLATFORM_WINDOWS
		delimiter = '\\';
#endif
		ea::vector<ea::string> file_parts;
		Atomic::string_split(module_path, delimiter, file_parts);
		const auto& file_name = file_parts[file_parts.size() - 1];

		bool is_json = false;
		if (Atomic::string_ends_with(file_name, ".json"))
			is_json = true;

		// If file isn't json and doesn't contain a .js extension
		// append .js extension at the end of file path.
		if (!is_json && !Atomic::string_ends_with(file_name, ".js"))
			module_path += ".js";

		const auto resource_cache = engine_ctx->GetResourceCache();
		return resource_cache->GetFile(module_path.c_str(), false);
	}

	duk_ret_t js_module_loader__try_load_file(duk_context* ctx, Atomic::Context* engine_ctx, const Atomic::SharedPtr<Atomic::File>& file)
	{
		const auto file_path = file->GetFullPath();
		const auto file_id = StringHash(file_path);

		duk_push_current_function(ctx);
		duk_get_prop_string(ctx, -1, JS_CACHE_PROP);

		// If user deletes 'cache' property, we must re-create
		if(duk_is_null_or_undefined(ctx, -1))
		{
			duk_pop(ctx);
			duk_push_object(ctx);
			duk_dup(ctx, -1);
			duk_put_prop_string(ctx, -3, JS_CACHE_PROP);
		}

		duk_remove(ctx, -2); // remove current 'require' function ref.
		if (duk_get_prop_index(ctx, -1, file_id.Value()))
		{
			duk_remove(ctx, -2); // remove cache object
			return 1;
		}


		// create module.exports object and put as global
		duk_push_object(ctx);
		duk_push_object(ctx);
		duk_put_prop_string(ctx, -2, "exports");
		duk_put_global_string(ctx, "module");

		{
			JS_ASSERT_HEAP(ctx);

			const auto script_sys = engine_ctx->GetSubsystem<IJavaScriptSystem>();
			if(!script_sys->Eval(file))
			{
				const auto err = ToString("Failed to load module %s", file_path.CString());
				duk_pop(ctx);
				duk_push_error_object(ctx, DUK_ERR_ERROR, err.CString());
				duk_throw(ctx);
			}
		}

		// get module.exports globally
		duk_get_global_string(ctx, "module");
		duk_get_prop_string(ctx, -1, "exports");
		duk_remove(ctx, -2); // remove 'module' ref

		const auto exports_idx = duk_get_top_index(ctx);

		duk_push_current_function(ctx);
		duk_get_prop_string(ctx, -1, JS_CACHE_PROP);
		duk_remove(ctx, -2); // remove 'require' function ref
		const auto cache_idx = duk_get_top_index(ctx);

		duk_dup(ctx, exports_idx);
		duk_dup(ctx, -1);
		duk_put_prop_index(ctx, cache_idx, file_id.Value()); // put exports into cache.

		duk_remove(ctx, cache_idx); // remove cache

		// remove module.exports from global
		duk_push_undefined(ctx);
		duk_put_global_string(ctx, "module");

		return 1;
	}

	void js_module_loader_setup(duk_context* ctx)
	{
		JS_ASSERT_HEAP(ctx);

		duk_push_global_object(ctx);
		duk_push_c_function(ctx, [](duk_context* ctx)
			{
				const ea::string module_path = duk_require_string(ctx, 0);
				duk_pop(ctx);

				const auto engine_ctx = JavaScriptSystem::GetEngineContext(ctx);
				const auto file = js_module_loader__load_file_module(ctx, engine_ctx, module_path);
				if(!file)
				{
					duk_push_error_object(ctx, DUK_ERR_ERROR, "Cannot find module '%s'", module_path.c_str());
					duk_throw(ctx);
				}

				return js_module_loader__try_load_file(ctx, engine_ctx, file);
			}, 1);
		duk_push_object(ctx);
		duk_put_prop_string(ctx, -2, JS_CACHE_PROP);
		duk_put_prop_string(ctx, -2, JS_REQUIRE_PROP);

		duk_pop(ctx);
	}
}
