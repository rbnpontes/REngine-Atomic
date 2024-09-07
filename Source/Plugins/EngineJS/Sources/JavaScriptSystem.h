#pragma once
#include <EngineCore/Core/Object.h>
#include <EngineCore/IO/File.h>
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
		bool Eval(const ea::string& js_code) override;
		bool EvalFromFilePath(const ea::string& file_path) override;
		bool Eval(SharedPtr<File> script_file) const;
		duk_context* GetHeap() const override { return js_context_; }
		size_t GetMemoryUsage() const override { return memory_usage_; }
		size_t GetMemoryBlocks() const override { return memory_blocks_; }
		static Context* GetEngineContext(duk_context* ctx);
	private:
		void SetupEngineContext() const;
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
