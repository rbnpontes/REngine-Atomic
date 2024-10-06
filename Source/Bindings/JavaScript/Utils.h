#pragma once
#include <Duktape/duktape.h>
#include <EngineCore/Core/Variant.h>
#include <EngineCore/Math/StringHash.h>

namespace REngine
{
	class heap_validator
	{
	public:
		heap_validator(duk_context* ctx, duk_uidx_t num);
		~heap_validator();
	private:
		duk_idx_t curr_top_;
		duk_uidx_t num_;
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

	enum class number_type
	{
		sign,
		unsign,
		decimal
	};

	void type_get(duk_context* ctx, const char* type_name, duk_size_t num_args, ...);
	void native_store_pointer(duk_context* ctx, duk_idx_t obj_idx, void* ptr);
	void* native_get_pointer(duk_context* ctx, duk_idx_t obj_idx);

	number_type number_get_type(duk_context* ctx, duk_idx_t num_idx);
}

#define assert_heap(ctx) \
	REngine::heap_validator __heap_validator(ctx, 0u)
#define assert_heap_num(ctx, num) \
	REngine::heap_validator __heap_validator(ctx, num)


#define using_global_namespace(ctx) \
	REngine::namespace_usage __global_ns(ctx)
#define using_namespace(ctx, ns_name) \
	REngine::namespace_usage __##ns_name(ctx, #ns_name)