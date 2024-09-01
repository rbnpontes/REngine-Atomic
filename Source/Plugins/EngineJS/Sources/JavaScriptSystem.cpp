#include "JavaScriptSystem.h"
#include <limits>

#include <EngineCore/IO/Log.h>
#include <EngineCore/Math/MathDefs.h>
#include <EngineCore/Resource/ResourceCache.h>

#include"./JavaScriptLogging.h"

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
	}

	void JavaScriptSystem::Initialize()
	{
		if (js_context_)
			return;
		js_context_ = CreateJsContext();
		// Setup default bindings
		js_setup_logging(js_context_);
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

		auto resource_cache = GetSubsystem<ResourceCache>();

		const auto script_file = resource_cache->GetFile(file_path.c_str());
		if (!script_file)
		{
			ATOMIC_CLASS_LOGERRORF(IJavaScriptSystem, "Not found JavaScript File %s", file_path.c_str());
			return true;
		}

		bool failed = false;

		const auto script_data = script_file->ReadText();
		duk_push_string(js_context_, script_file->GetFullPath().CString());
		if (duk_pcompile_string_filename(js_context_, 0, script_data.CString()) != 0)
		{
			const char* err_msg = duk_safe_to_string(js_context_, -1);
			ATOMIC_CLASS_LOGERROR(IJavaScriptSystem, err_msg);
			failed = true;
		}
		else
			duk_call(js_context_, 0);

		duk_pop(js_context_);

		return failed;
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
			delete result;
			return AllocMemory(udata, size);
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
