#include "Utils.h"
#include <cassert>
#include <cmath>

#define NATIVE_POINTER_PROP DUK_HIDDEN_SYMBOL("ptr")
namespace REngine
{
	heap_validator::heap_validator(duk_context* ctx, duk_uidx_t num) : ctx_(ctx), num_(num)
	{
		curr_top_ = duk_get_top(ctx);
	}
	heap_validator::~heap_validator()
	{
		const auto top = duk_get_top(ctx_);
		const auto diff = top - curr_top_;
			assert(diff == num_ && "Invalid heap size. Check your implementation! Probably you forget to do some duk_pop call.");
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

	void type_get(duk_context* ctx, const char* type_name, duk_size_t num_args, ...)
	{
		va_list args;
		va_start(args, num_args);

		const auto top = duk_get_top(ctx);
		const auto top_idx = top - 1;
		duk_push_global_object(ctx);
		for(duk_size_t i = 0; i < num_args; ++i)
		{
			const char* ns = va_arg(i, const char*);
			if(duk_get_prop_string(ctx, -1, ns))
				continue;

			duk_pop(ctx);
			duk_push_object(ctx);
			duk_dup(ctx, -1);
			duk_put_prop_string(ctx, -3, ns);
		}
		va_end(args);

		if(!duk_get_prop_string(ctx, -1,type_name))
		{
			duk_pop_n(ctx, duk_get_top(ctx) - top);

			duk_push_error_object(ctx, DUK_ERR_ERROR, "Not found type %s", type_name);
			duk_throw(ctx);
		}

		duk_replace(ctx, top_idx);
		duk_pop_n(ctx, duk_get_top_index(ctx) - top);
	}

	void native_store_pointer(duk_context* ctx, duk_idx_t obj_idx, void* ptr)
	{
		obj_idx = duk_normalize_index(ctx, obj_idx);
		duk_push_pointer(ctx, ptr);
		duk_put_prop_string(ctx, obj_idx, NATIVE_POINTER_PROP);
	}
	void* native_get_pointer(duk_context* ctx, duk_idx_t obj_idx)
	{
		obj_idx = duk_normalize_index(ctx, obj_idx);
		void* result = nullptr;
		if(duk_get_prop_string(ctx, obj_idx, NATIVE_POINTER_PROP))
		{
			result = duk_get_pointer_default(ctx, -1, nullptr);
			duk_pop(ctx);
		}

		return result;
	}
	number_type number_get_type(duk_context* ctx, duk_idx_t num_idx)
	{
		const auto value = duk_get_number(ctx, num_idx);
		const auto floor_val = static_cast<uint64_t>(floor(value));
		const auto is_int = std::isfinite(value) && floor_val == static_cast<uint64_t>(value);
		const auto is_unsigned = value > 0;

		if (is_int)
			return is_unsigned ? number_type::unsign : number_type::sign;
		return number_type::decimal;
	}
}
