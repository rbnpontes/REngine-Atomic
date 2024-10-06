#include "JavaScriptVariant.h"
#include "JavaScriptAssert.h"
#include <EngineCore/Core/Variant.h>
#include <limits>

namespace REngine
{
	void js_variant__fill_number(Atomic::Variant* var, double value)
	{
		if(isnan(value))
		{
			*var = value;
			return;
		}

		// test if value is an integer
		const auto floor_value = static_cast<u64>(floor(value));
		const auto rounded_value = static_cast<u64>(round(value));

		if (floor_value == rounded_value)
		{
			constexpr auto min_int = static_cast<u64>(std::numeric_limits<u32>::lowest());
			constexpr auto max_int = static_cast<u64>(std::numeric_limits<u32>::max());
			if (floor_value >= min_int && value <= max_int)
				*var = static_cast<u32>(floor_value);
			else
				*var = floor_value;
			return;
		}

		constexpr auto min_float = static_cast<double>(std::numeric_limits<float>::lowest());
		constexpr auto max_float = static_cast<double>(std::numeric_limits<float>::max());
		if (value >= min_float && value <= max_float)
			*var = static_cast<float>(value);
		else
			*var = value;
	}
	duk_idx_t js_variant__ctor(duk_context* ctx)
	{
		if(!duk_is_constructor_call(ctx))
		{
			duk_push_error_object(ctx, DUK_ERR_ERROR, "Class constructor Variant cannot be invoked without 'new'");
			return duk_throw(ctx);
		}

		const auto instance = new Atomic::Variant();
		if (duk_is_pointer(ctx, 0))
			*instance = duk_get_pointer_default(ctx, 0, nullptr);
		else if (duk_is_number(ctx, 0))
			js_variant__fill_number(instance, duk_get_number(ctx, 0));
		else if (duk_is_string(ctx, 0))
			*instance = Atomic::String(duk_get_string_default(ctx, 0, ""));
		else if (duk_is_boolean(ctx, 0))
			*instance = duk_get_boolean_default(ctx, 0, false);
		else if(duk_is_array(ctx, 0)){}
		else if(duk_is_object(ctx, 0)) {}
		else
		{
			delete instance;
			duk_push_error_object(ctx, DUK_ERR_ERROR, "Unsupported value type at Variant constructor call.");
			return duk_throw(ctx);
		}
		return 0;
	}

	void js_variant_setup(duk_context* ctx)
	{
		JS_ASSERT_HEAP(ctx);

		duk_push_global_object(ctx);
		duk_push_c_function(ctx, js_variant__ctor, DUK_VARARGS);
		duk_put_prop_string(ctx, -2, "Variant");
		duk_pop(ctx);
	}

}