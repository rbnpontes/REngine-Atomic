#pragma once
#include <Duktape/duktape.h>

namespace REngine
{
	void js_timers_setup(duk_context* ctx);
	void js_timers_exec_pending_timers(duk_context* ctx);
	bool js_timers_has_pending_timers(duk_context* ctx);
	void js_timers_clear_timers(duk_context* ctx);
}
