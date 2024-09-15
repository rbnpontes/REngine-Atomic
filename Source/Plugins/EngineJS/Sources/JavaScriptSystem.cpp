#include "JavaScriptSystem.h"
#include <limits>

#include <EngineCore/IO/Log.h>
#include <EngineCore/Math/MathDefs.h>
#include <EngineCore/Resource/ResourceCache.h>
#include <EngineCore/Core/CoreEvents.h>

#include "./JavaScriptLogging.h"
#include "./JavaScriptModuleLoader.h"
#include "./JavaScriptTimers.h"
#include "./JavaScriptAssert.h"
#include "EngineCore/IO/FileSystem.h"

#define ENGINE_CONTEXT_PROP "engine_ctx"
#define ENGINE_DIRNAME_PROP "__dirname"
#define ENGINE_FILENAME_PROP "__filename"
namespace REngine
{
	static void assert_context(duk_context* js_ctx)
	{
		assert(js_ctx && "JavaScriptSystem is not initialized.");
	}

	JavaScriptSystem::JavaScriptSystem(Context* context): Object(context),
		js_context_(nullptr),
		memory_usage_(0),
		memory_blocks_(0)
	{
		SubscribeToEvent(E_BEGINFRAME, ATOMIC_HANDLER(JavaScriptSystem, OnBeginFrame));
	}

	void JavaScriptSystem::Initialize()
	{
		if (js_context_)
			return;
		js_context_ = CreateJsContext();

		SetupEngineContext();
		// Setup default bindings
		js_logging_setup(js_context_);
		js_module_loader_setup(js_context_);
		js_timers_setup(js_context_);
	}

	bool JavaScriptSystem::Eval(const ea::string& js_code)
	{
		assert_context(js_context_);
		bool failed = false;

		if(duk_peval_lstring(js_context_, js_code.c_str(), js_code.length()) != 0)
		{
			const char* err_msg = duk_safe_to_string(js_context_, -1);
			ATOMIC_CLASS_LOGERROR(JavaScriptSystem, err_msg);
			failed = true;
		}

		duk_pop(js_context_);

		return failed;
	}

	bool JavaScriptSystem::EvalFromFilePath(const ea::string& file_path)
	{
		assert_context(js_context_);
		JS_ASSERT_HEAP(js_context_);

		auto resource_cache = GetSubsystem<ResourceCache>();

		const auto script_file = resource_cache->GetFile(file_path.c_str());
		if (!script_file)
		{
			ATOMIC_CLASS_LOGERRORF(IJavaScriptSystem, "Not found JavaScript File %s", file_path.c_str());
			return true;
		}

		return Eval(script_file);
	}

	bool JavaScriptSystem::Eval(SharedPtr<File> script_file) const
	{
		assert_context(js_context_);
		assert(script_file);
		JS_ASSERT_HEAP(js_context_);

		bool success = true;
		const auto script_path = script_file->GetFullPath();
		const auto script_data = script_file->ReadText();

		String path_name;
		String file_name;
		String ext;
		Atomic::SplitPath(
			script_path, 
			path_name, 
			file_name, 
			ext);
		const auto script_name_w_ext = file_name + ext;

		ea::string prev_dir_name_val, prev_file_name_val;

		if(duk_get_global_string(js_context_, ENGINE_DIRNAME_PROP))
		{
			const auto val = duk_get_string(js_context_, -1);
			if (val)
				prev_dir_name_val = val;
		}

		if(duk_get_global_string(js_context_, ENGINE_FILENAME_PROP))
		{
			const auto val = duk_get_string(js_context_, -1);
			if (val)
				prev_file_name_val = val;
		}
		// remove __dirname and __filename refs
		duk_pop_2(js_context_);

		// add required global values
		duk_push_string(js_context_, path_name.CString());
		duk_put_global_string(js_context_, ENGINE_DIRNAME_PROP);

		duk_push_string(js_context_, script_name_w_ext.CString());
		duk_put_global_string(js_context_, ENGINE_FILENAME_PROP);

		duk_push_string(js_context_, script_path.CString());
		if (duk_pcompile_string_filename(js_context_, 0, script_data.CString()) != 0)
		{
			const char* err_msg = duk_safe_to_string(js_context_, -1);
			ATOMIC_CLASS_LOGERROR(IJavaScriptSystem, err_msg);
			success = false;
		}
		else
			duk_call(js_context_, 0);

		duk_pop(js_context_);

		// put it back default global properties
		if(prev_dir_name_val.empty())
			duk_push_undefined(js_context_);
		else
			duk_push_string(js_context_, prev_dir_name_val.c_str());
		duk_put_global_string(js_context_, ENGINE_DIRNAME_PROP);

		if (prev_file_name_val.empty())
			duk_push_undefined(js_context_);
		else
			duk_push_string(js_context_, prev_file_name_val.c_str());
		duk_put_global_string(js_context_, ENGINE_FILENAME_PROP);

		return success;
	}

	void JavaScriptSystem::ExecutePendingTimers()
	{
		assert_context(js_context_);
		js_timers_exec_pending_timers(js_context_);
	}

	void JavaScriptSystem::ClearAllTimers()
	{
		assert_context(js_context_);
		js_timers_clear_timers(js_context_);
	}

	bool JavaScriptSystem::HasPendingTimers()
	{
		if (!js_context_)
			return false;
		return js_timers_has_pending_timers(js_context_);
	}

	Context* JavaScriptSystem::GetEngineContext(duk_context* ctx)
	{
		JS_ASSERT_HEAP(ctx);

		duk_push_global_stash(ctx);
		duk_get_prop_string(ctx, -1, ENGINE_CONTEXT_PROP);

		Context* engine_ctx = nullptr;
		if (duk_is_pointer(ctx, -1))
			engine_ctx = static_cast<Context*>(duk_get_pointer(ctx, -1));

		duk_pop_2(ctx);
		return engine_ctx;
	}

	void JavaScriptSystem::OnBeginFrame(StringHash event_type, VariantMap& event_args)
	{
		if (js_context_)
			return;

		if (HasPendingTimers())
			ExecutePendingTimers();
	}

	void JavaScriptSystem::SetupEngineContext() const
	{
		JS_ASSERT_HEAP(js_context_);

		duk_push_global_stash(js_context_);
		duk_push_pointer(js_context_, context_);
		duk_put_prop_string(js_context_, -2, ENGINE_CONTEXT_PROP);

		duk_pop(js_context_);
	}

	void* JavaScriptSystem::AllocMemory(void* udata, duk_size_t length)
	{
		assert(length < std::numeric_limits<u32>::max() && "Failed to allocate JavaScript memory");

		if (!length)
			return nullptr;

		const auto instance = static_cast<JavaScriptSystem*>(udata);
		instance->memory_usage_ += length;
		++instance->memory_blocks_;

		char* result = new char[sizeof(u32) + length];
		// store memory length at head of allocated memory
		u32* mem_size = static_cast<u32*>(static_cast<void*>(result));
		*mem_size = static_cast<u32>(length);

		return result + sizeof(u32);
	}

	void* JavaScriptSystem::ReAllocMemory(void* udata, void* ptr, duk_size_t size)
	{
		if (!ptr)
			return AllocMemory(udata, size);

		//// shift 4 bytes(sizeof u32) to get allocated memory length
		char* result = static_cast<char*>(ptr) - sizeof(u32);

		const u32 alloc_size = *static_cast<u32*>(static_cast<void*>(result));

		const auto instance = static_cast<JavaScriptSystem*>(udata);
		instance->memory_usage_ -= alloc_size;
		--instance->memory_blocks_;
		
		// if duktape requests more memory than allocated, we must
		// discard previous memory and allocate a new one.
		if(size > alloc_size)
		{
			void* new_ptr = AllocMemory(udata, size);
			// copy current data to new address
			memcpy(new_ptr, result, size);
			delete result;
			return new_ptr;
		}

		// if duktape requests less memory than previous allocated
		// returns same memory.
		instance->memory_usage_ += alloc_size;
		++instance->memory_blocks_;
		return result + sizeof(u32);
	}

	void JavaScriptSystem::DeAllocMemory(void* udata, void* ptr)
	{
		if (!ptr)
			return;

		char* result = static_cast<char*>(ptr) - sizeof(u32);
		const u32 alloc_size = *static_cast<u32*>(static_cast<void*>(result));

		delete result;

		const auto instance = static_cast<JavaScriptSystem*>(udata);
		instance->memory_usage_ -= alloc_size;
		--instance->memory_blocks_;
	}

	void JavaScriptSystem::HandleFatalError(void* udata, const char* msg)
	{
		ATOMIC_CLASS_LOGERROR(JavaScriptSystem, msg);
		const auto instance = static_cast<JavaScriptSystem*>(udata);
		instance->js_context_ = nullptr;
		instance->Initialize();
	}

	duk_context* JavaScriptSystem::CreateJsContext()
	{
		return duk_create_heap(
			AllocMemory,
			ReAllocMemory,
			DeAllocMemory,
			this,
			HandleFatalError
		);
	}

}
