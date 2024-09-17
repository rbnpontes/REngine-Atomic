#include "JavaScriptLogging.h"
#include "JsonBuilder.h"
#include <EngineCore/IO/Log.h>
#include <cstdlib>

#define MAX_LOG_DEPTH 3
namespace REngine
{
	void js__build_log_array(duk_context* ctx, ea::shared_ptr<JsonArray>& array, duk_idx_t arr_2_read_idx, u8 depth);
	void js__build_log_obj(duk_context* ctx, ea::shared_ptr<JsonObject>& obj, duk_idx_t obj_2_read_idx, u8 depth);


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
	void js__fill_primitive(duk_context* ctx, duk_idx_t idx, ea::shared_ptr<JsonPrimitive>& primitive)
	{
		if (duk_is_string(ctx, idx))
			primitive->SetValue(duk_get_string(ctx, idx));
		else if (duk_is_function(ctx, idx))
			primitive->SetValue(duk_to_string(ctx, idx));
		else if (duk_is_boolean(ctx, idx))
			primitive->SetValue(static_cast<bool>(duk_is_boolean(ctx, idx)));
		else if (duk_is_pointer(ctx, idx))
			primitive->SetValue(duk_to_string(ctx, idx));
		else if (duk_is_nan(ctx, idx))
			primitive->SetValue("NaN");
		else if (duk_is_null(ctx, idx))
			primitive->SetValue("null");
		else if (duk_is_undefined(ctx, idx))
			primitive->SetValue("undefined");
		else if (duk_is_number(ctx, idx))
			primitive->SetValue(duk_to_number(ctx, idx));
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

	void js__build_log_array(duk_context* ctx, ea::shared_ptr<JsonArray>& array, duk_idx_t arr_2_read_idx, u8 depth)
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
				auto primitive = array->PushPrimitive();
				js__fill_primitive(ctx, value_idx, primitive);
			}
			else if (duk_is_array(ctx, value_idx))
			{
				auto child_array = array->PushArray();
				js__build_log_array(ctx, child_array, value_idx, depth + 1);
			}
			else if (duk_is_object(ctx, value_idx))
			{
				auto obj = array->PushObject();
				js__build_log_obj(ctx, obj, value_idx, depth + 1);
			}

			duk_pop(ctx);
		}
	}
	void js__build_log_obj(duk_context* ctx, ea::shared_ptr<JsonObject>& obj, duk_idx_t obj_2_read_idx, u8 depth)
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
				auto primitive = obj->AddPrimitive(prop_name);
				js__fill_primitive(ctx, value_idx, primitive);
			}
			else if (duk_is_array(ctx, value_idx))
			{
				auto arr = obj->AddArray(prop_name);
				js__build_log_array(ctx, arr, value_idx, depth + 1);
			}
			else if (duk_is_object(ctx, value_idx))
			{
				auto child_obj = obj->AddObject(prop_name);
				js__build_log_obj(ctx, child_obj, value_idx, depth + 1);
			}

			duk_pop_2(ctx);
		}

		duk_pop(ctx);
	}

	void js__log(duk_context* ctx, int log_lvl)
	{
		const auto args = duk_get_top(ctx);

		for(duk_idx_t i =0; i < args; ++i)
		{
			ea::string message;
			if (js__is_non_loggable_value(ctx, i))
				message = js__get_default_string(ctx, i);
			if (duk_is_array(ctx, -1))
			{
				auto arr = JsonBuilder::GetArray();
				js__build_log_array(ctx, arr, i, 0);
				message = arr->ToString();
			}
			else if (duk_is_object(ctx, i))
			{
				auto obj = JsonBuilder::GetObject();
				js__build_log_obj(ctx, obj, i, 0);
				message = obj->ToString();
			}
			Atomic::Log::Write(log_lvl, message.c_str());
		}
	}

	void js_logging_setup(duk_context* ctx)
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

		duk_push_c_function(ctx, [](duk_context* ctx)
		{
			js__log(ctx, Atomic::LOG_SUCCESS);
			return 0;
		}, DUK_VARARGS);
		duk_put_prop_string(ctx, -2, "success");

		duk_push_c_function(ctx, [](duk_context* ctx)
		{
			js__log(ctx, Atomic::LOG_RAW);
			return 0;
		}, DUK_VARARGS);
		duk_put_prop_string(ctx, -2, "raw");

		duk_push_c_lightfunc(ctx, [](duk_context* ctx)
		{
#ifdef ENGINE_PLATFORM_WINDOWS
			std::system("cls");
#else
			std::system("clear");
#endif
			ATOMIC_LOGDEBUG("Console was cleared");
			return 0;
		}, 0, 0, 0);
		duk_put_prop_string(ctx, -2, "clear");

		duk_put_global_string(ctx, "console");
	}

}
