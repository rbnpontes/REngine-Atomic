#pragma once
#include <EngineCore/Core/Object.h>
#include <Plugins/EngineJS/Interfaces/IJavaScriptSystem.h>
#include <Duktape/duktape.h>

namespace REngine
{
	using namespace Atomic;
	class JavaScriptSystem : public Object, public IJavaScriptSystem
	{
		ATOMIC_OBJECT(JavaScriptSystem, Object);
	public:
		JavaScriptSystem(Context* context);
		void Initialize();
		void Eval(const ea::string& js_code) override;
		void EvalFromFilePath(const ea::string& file_path) override;
		size_t GetMemoryUsage() const { return memory_usage_; }
		size_t GetMemoryBlocks() const { return memory_blocks_; }
	private:
		static void* AllocMemory(void* udata, duk_size_t length);
		static void* ReAllocMemory(void* udata, void* ptr, duk_size_t size);
		static void DeAllocMemory(void* udata, void* ptr);
		static void HandleFatalError(void* udata, const char* msg);
		duk_context* CreateJsContext();
		duk_context* js_context_;

		size_t memory_usage_;
		size_t memory_blocks_;
	};
}
