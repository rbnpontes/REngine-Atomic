#pragma once
#include <Duktape/duktape.h>
namespace REngine
{
	void bookkeeper_store(duk_context* ctx, duk_idx_t obj_idx, void* native_obj);
	duk_bool_t bookkeeper_restore(duk_context* ctx, void* ptr);
	void bookkeeper_remove(duk_context* ctx, void* ptr);
	duk_bool_t bookkeeper_is_valid(duk_context* ctx, void* ptr);
}
