#pragma once
#include <assert.h>
#include <Duktape/duktape.h>
#define JS_BEGIN_OP(ctx) \
	duk_idx_t __begin_curr_heap = duk_get_top(ctx)
#define JS_END_OP(ctx) \
	duk_idx_t __end_curr_heap = duk_get_top(ctx); \
	assert(__begin_curr_heap == __end_curr_heap && "Invalid heap size. Check your implementation! Probably you forget to do some duk_pop call.")

class js_heap_validator
{
public:
	js_heap_validator(duk_context* ctx)
	{
		ctx_ = ctx;
		curr_top_ = duk_get_top(ctx);
	}
	~js_heap_validator()
	{
		const auto top = duk_get_top(ctx_);
		assert(curr_top_ == top && "Invalid heap size. Check your implementation! Probably you forget to do some duk_pop call.");
	}
private:
	duk_idx_t curr_top_;
	duk_context* ctx_;
};

#define JS_ASSERT_HEAP(ctx) \
	js_heap_validator __heap_validator(ctx)