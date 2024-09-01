#include "JavaScriptLogging.h"
#include <EngineCore/IO/Log.h>

#define MAX_LOG_DEPTH 3
namespace REngine
{
	void js__build_log_array(duk_context* ctx, duk_idx_t parent_idx, duk_idx_t arr_2_read_idx, u8 depth);
	void js__build_log_obj(duk_context* ctx, duk_idx_t parent_idx, duk_idx_t obj_2_read_idx, u8 depth);


	bool js__is_primitive(duk_context* ctx, duk_idx_t idx)
	{
		idx = duk_require_normalize_index(ctx, idx);
		if (duk_is_nan(ctx, idx))
			return false;
		return duk_is_string(ctx, idx) || duk_is_boolean(ctx, idx) || duk_is_number(ctx, idx);
	} 
	bool js__is_non_loggable_value(duk_context* ctx, duk_idx_t idx)
	{
		idx = duk_require_normalize_index(ctx, idx);
		return js__is_primitive(ctx, idx) || duk_is_function(ctx, idx)
			|| duk_is_nan(ctx, idx) || duk_is_null_or_undefined(ctx, idx);
	}
	ea::string js__get_default_string(duk_context* ctx, duk_idx_t idx)
	{
		if (duk_is_string(ctx, idx))
			return duk_get_string(ctx, idx);
		if (duk_is_function(ctx, idx) || duk_is_number(ctx, idx))
			return duk_to_string(ctx, idx);
		if (duk_is_boolean(ctx, idx))
			return duk_get_boolean(ctx, idx) ? "true" : "false";
		if (duk_is_nan(ctx, idx))
			return"NaN";
		if (duk_is_null(ctx, idx))
			return "null";
		if (duk_is_undefined(ctx, idx))
			return "undefined";
		return "";
	}

	ea::string js__stringify(duk_context* ctx, duk_idx_t obj_idx)
	{
		obj_idx = duk_require_normalize_index(ctx, obj_idx);
		const auto heap_top = duk_get_top(ctx);

		duk_get_global_string(ctx, "JSON");
		duk_push_string(ctx, "stringify");
		duk_dup(ctx, obj_idx);
		duk_push_null(ctx);
		duk_push_string(ctx, "\t");
		duk_call_prop(ctx, -5, 3);

		ea::string result = duk_get_string(ctx, -1);
		duk_pop_n(ctx, duk_get_top(ctx) - heap_top);

		return result;
	}

	void js__build_log_array(duk_context* ctx, duk_idx_t parent_idx, duk_idx_t arr_2_read_idx, u8 depth)
	{
		// Skip if depth is too long
		if (depth >= MAX_LOG_DEPTH)
			return;

		const auto arr_len = duk_get_length(ctx, arr_2_read_idx);
		for(duk_uarridx_t i = 0; i < arr_len; ++i)
		{
			duk_get_prop_index(ctx, arr_2_read_idx, i);
			const auto value_idx = duk_get_top_index(ctx);

			if (js__is_non_loggable_value(ctx, value_idx))
			{
				if (js__is_primitive(ctx, value_idx))
					duk_dup(ctx, value_idx);
				else
					duk_push_string(ctx, js__get_default_string(ctx, value_idx).c_str());
			}
			else if (duk_is_array(ctx, value_idx))
			{
				duk_push_array(ctx);
				js__build_log_array(ctx, duk_get_top_index(ctx), value_idx, depth + 1);
			}
			else if (duk_is_object(ctx, value_idx))
			{
				duk_push_object(ctx);
				js__build_log_obj(ctx, duk_get_top_index(ctx), value_idx, depth + 1);
			}
			else
				duk_push_undefined(ctx);

			duk_put_prop_index(ctx, parent_idx, i);

			duk_pop(ctx);
		}
	}
	void js__build_log_obj(duk_context* ctx, duk_idx_t parent_idx, duk_idx_t obj_2_read_idx, u8 depth)
	{
		// Skip if depth is too long
		if (depth >= MAX_LOG_DEPTH)
			return;

		duk_enum(ctx, obj_2_read_idx, DUK_ENUM_OWN_PROPERTIES_ONLY);
		while(duk_next(ctx, -1, true))
		{
			ea::string prop_name = duk_get_string(ctx, -2);
			const auto value_idx = duk_get_top_index(ctx);

			if (js__is_non_loggable_value(ctx, value_idx))
			{
				if (js__is_primitive(ctx, value_idx))
					duk_dup(ctx, value_idx);
				else
					duk_push_string(ctx, js__get_default_string(ctx, value_idx).c_str());
			}
			else if (duk_is_array(ctx, value_idx))
			{
				duk_push_array(ctx);
				js__build_log_obj(ctx, duk_get_top_index(ctx), value_idx, depth + 1);
			}
			else if (duk_is_object(ctx, value_idx))
			{
				duk_push_object(ctx);
				js__build_log_obj(ctx, duk_get_top_index(ctx), value_idx, depth + 1);
			}
			else
				duk_push_undefined(ctx);

			duk_put_prop_string(ctx, parent_idx, prop_name.c_str());

			duk_pop_2(ctx);
		}

		duk_pop(ctx);
	}
	ea::string js__get_obj_str(duk_context* ctx, duk_idx_t idx)
	{
		const auto heap_top = duk_get_top(ctx);
		const auto obj_idx = duk_get_top(ctx);

		duk_push_object(ctx);

		js__build_log_obj(ctx, obj_idx, idx, 0);

		ea::string result = js__stringify(ctx, obj_idx);

		duk_pop(ctx);

		const auto curr_heap_top = duk_get_top(ctx);
		assert(curr_heap_top == heap_top);

		return result;
	}
	ea::string js__get_arr_str(duk_context* ctx, duk_idx_t idx)
	{
		const auto heap_top = duk_get_top(ctx);
		const auto arr_idx = duk_get_top(ctx);
		duk_push_array(ctx);

		js__build_log_array(ctx, arr_idx, idx, 0);

		duk_get_global_string(ctx, "JSON");
		duk_push_string(ctx, "stringify");
		duk_dup(ctx, arr_idx);
		duk_push_null(ctx);
		duk_push_string(ctx, "\t");
		duk_call_prop(ctx, -5, 3);

		ea::string result = duk_get_string(ctx, -1);
		duk_pop_3(ctx);

		const auto curr_heap_top = duk_get_top(ctx);
		assert(curr_heap_top == heap_top);

		return result;
	}

	void js__log(duk_context* ctx, int log_lvl)
	{
		const auto args = duk_get_top(ctx);

		for(duk_idx_t i =0; i < args; ++i)
		{
			if(js__is_non_loggable_value(ctx, i))
				Atomic::Log::Write(log_lvl, js__get_default_string(ctx, i).c_str());
			if (duk_is_array(ctx, -1))
				Atomic::Log::Write(log_lvl, js__get_arr_str(ctx, i).c_str());
			else if (duk_is_object(ctx, i))
				Atomic::Log::Write(log_lvl, js__get_obj_str(ctx, i).c_str());
		}
	}

	void js_setup_logging(duk_context* ctx)
	{
		duk_push_object(ctx);

		duk_push_c_function(ctx, [](duk_context* ctx)
		{
			js__log(ctx, Atomic::LOG_DEBUG);
			return 0;
		}, DUK_VARARGS);
		duk_dup(ctx, -1);
		duk_put_prop_string(ctx, -3, "log");
		duk_put_prop_string(ctx, -2, "debug");

		duk_push_c_function(ctx, [](duk_context* ctx)
		{
			js__log(ctx, Atomic::LOG_INFO);
			return 0;
		}, DUK_VARARGS);
		duk_put_prop_string(ctx, -2, "info");

		duk_push_c_function(ctx, [](duk_context* ctx)
		{
			js__log(ctx, Atomic::LOG_WARNING);
			return 0;
		}, DUK_VARARGS);
		duk_put_prop_string(ctx, -2, "warn");

		duk_push_c_function(ctx, [](duk_context* ctx)
		{
			js__log(ctx, Atomic::LOG_ERROR);
			return 0;
		}, DUK_VARARGS);
		duk_put_prop_string(ctx, -2, "error");

		duk_put_global_string(ctx, "console");
	}

}
