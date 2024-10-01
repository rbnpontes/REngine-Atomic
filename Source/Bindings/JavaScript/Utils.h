#pragma once
#include <Duktape/duktape.h>
#include <assert.h>

namespace REngine
{
	class heap_validator
	{
	public:
		heap_validator(duk_context* ctx);
		~heap_validator();
	private:
		duk_idx_t curr_top_;
		duk_context* ctx_;
	};

	class namespace_usage
	{
	public:
		namespace_usage(duk_context* ctx);
		namespace_usage(duk_context* ctx, const char* ns);
		~namespace_usage();
	private:
		void acquire_global_namespace();
		void acquire_namespace(const char* ns_name);
		duk_context* ctx_;
	};	
}

#define assert_heap(ctx) \
	REngine::heap_validator __heap_validator(ctx)

#define using_global_namespace(ctx) \
	REngine::namespace_usage __global_ns(ctx)
#define using_namespace(ctx, ns_name) \
	REngine::namespace_usage __##ns_name(ctx, #ns_name)