#include "Utils.h"

namespace REngine
{
	heap_validator::heap_validator(duk_context* ctx) : ctx_(ctx)
	{
		curr_top_ = duk_get_top(ctx);
	}
	heap_validator::~heap_validator()
	{
		const auto top = duk_get_top(ctx_);
			assert(curr_top_ == top && "Invalid heap size. Check your implementation! Probably you forget to do some duk_pop call.");
	}

	namespace_usage::namespace_usage(duk_context* ctx) : ctx_(ctx)
	{
		acquire_global_namespace();
	}

	namespace_usage::namespace_usage(duk_context*ctx, const char* ns) : ctx_(ctx)
	{
		acquire_namespace(ns);
	}

	namespace_usage::~namespace_usage()
	{
		duk_pop(ctx_);
	}

	void namespace_usage::acquire_global_namespace()
	{
		duk_push_global_object(ctx_);
	}
	void namespace_usage::acquire_namespace(const char* ns_name)
	{
		duk_get_prop_string(ctx_, -1, ns_name);
		if(duk_is_null_or_undefined(ctx_, -1))
		{
			duk_pop(ctx_);
			duk_push_object(ctx_);
			duk_dup(ctx_, -1);
			duk_put_prop_string(ctx_, -3, ns_name);
		}
	}
}
