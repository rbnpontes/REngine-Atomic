#include "JavaScriptTimers.h"
#include "JavaScriptAssert.h"
#include <EngineCore/Container/TypeTraits.h>

#define JS_TIMERS_PROP "timers"
#define JS_TIMERS_CTX_PROP "ctx"
#define JS_TIMERS_CALLBACKS_PROP "callbacks"
namespace REngine
{
	enum class timer_type
	{
		immediate	= 0,
		timeout		= 1,
		interval	= 2
	};
	struct timeout_desc
	{
		duk_double_t timeout_;
		duk_double_t interval_;
		bool is_interval_;
		bool ignore_;
		void* callback_;
	};

	struct timers_context
	{
		ea::queue<ea::shared_ptr<timeout_desc>> timers_;
	};

	timers_context* js_timers__get_context(duk_context* ctx)
	{
		JS_ASSERT_HEAP(ctx);

		duk_push_global_object(ctx);
		duk_get_prop_string(ctx, -1, JS_TIMERS_PROP);

		if(duk_is_null_or_undefined(ctx, -1))
		{
			duk_pop_2(ctx);
			return nullptr;
		}

		duk_get_prop_string(ctx, -1, JS_TIMERS_CTX_PROP);
		if (!duk_is_pointer(ctx, -1))
		{
			duk_pop_3(ctx);
			return nullptr;
		}

		const auto result = static_cast<timers_context*>(duk_get_pointer(ctx, -1));
		duk_pop_n(ctx, 4);
		return result;
	}

	void js_timers__store_callback(duk_context* ctx, void* id, duk_idx_t func_idx)
	{
		JS_ASSERT_HEAP(ctx);

		duk_push_global_stash(ctx);
		duk_get_prop_string(ctx, -1, JS_TIMERS_PROP);

		if(duk_is_null_or_undefined(ctx, -1))
		{
			duk_pop_2(ctx);
			return;
		}

		duk_get_prop_string(ctx, -1, JS_TIMERS_CALLBACKS_PROP);
		if(duk_is_null_or_undefined(ctx, -1))
		{
			duk_pop_3(ctx);
			return;
		}

		duk_push_pointer(ctx, id);
		duk_dup(ctx, func_idx);

		duk_put_prop(ctx,  -3);
		duk_pop_3(ctx);
	}
	void js_timers__remove_callback(duk_context* ctx, void* id)
	{
		JS_ASSERT_HEAP(ctx);
		duk_push_global_stash(ctx);
		duk_get_prop_string(ctx, -1, JS_TIMERS_PROP);
		duk_get_prop_string(ctx, -1, JS_TIMERS_CALLBACKS_PROP);
		duk_push_pointer(ctx, id);
		duk_del_prop(ctx, -2);
		duk_pop_3(ctx);
	}

	bool js_timers__is_valid_timer(duk_context* ctx, void* id)
	{
		JS_ASSERT_HEAP(ctx);

		duk_push_global_stash(ctx);
		duk_get_prop_string(ctx, -1, JS_TIMERS_PROP);

		if(duk_is_null_or_undefined(ctx, -1))
		{
			duk_pop_2(ctx);
			return false;
		}

		duk_get_prop_string(ctx, -1, JS_TIMERS_CALLBACKS_PROP);
		if(duk_is_null_or_undefined(ctx, -1))
		{
			duk_pop_3(ctx);
			return false;
		}

		duk_push_pointer(ctx, id);
		const auto res = duk_get_prop(ctx, -2);
		duk_pop_n(ctx, 4);

		return res;
	} 

	duk_ret_t js_timers__create_timeout(duk_context* ctx, timer_type type)
	{
		duk_require_function(ctx, 0);
		duk_uidx_t timeout = 0;
		if(type == timer_type::immediate)
		{
			timeout = duk_require_uint(ctx, 1);
			duk_pop(ctx);
		}

		timers_context* t_ctx = js_timers__get_context(ctx);
		ea::shared_ptr<timeout_desc> desc(new timeout_desc());
		desc->ignore_ = false;
		desc->callback_ = duk_get_heapptr(ctx, 0);
		desc->timeout_ = timeout;
		if (type != timer_type::immediate)
			desc->timeout_ += duk_get_now(ctx);
		desc->is_interval_ = type == timer_type::interval;

		t_ctx->timers_.push(desc);
		js_timers__store_callback(ctx, desc.get(), 0);

		duk_push_pointer(ctx, desc.get());
		return 1;
	}
	duk_ret_t js_timers__clear_timeout(duk_context* ctx)
	{
		const auto id = duk_require_pointer(ctx, 0);

		if(!js_timers__is_valid_timer(ctx, id))
			return 0;

		auto timer = static_cast<timeout_desc*>(id);
		timer->ignore_ = true;
		return 0;
	}

	void js_timers_setup(duk_context* ctx)
	{
		JS_ASSERT_HEAP(ctx);
		// setup setImmediate call
		duk_push_c_lightfunc(ctx, [](duk_context* ctx)
		{
			return js_timers__create_timeout(ctx, timer_type::interval);
		}, 1, 1, 0);
		duk_put_global_string(ctx, "setImmediate");
		// setup setTimeout call
		duk_push_c_lightfunc(ctx, [](duk_context* ctx)
		{
			return js_timers__create_timeout(ctx, timer_type::timeout);
		}, 2, 2, 0);
		duk_put_global_string(ctx, "setTimeout");
		// setup setInterval call
		duk_push_c_lightfunc(ctx, [](duk_context* ctx)
		{
			return js_timers__create_timeout(ctx, timer_type::interval);
		}, 2, 2, 0);
		duk_put_global_string(ctx, "setInterval");
		// setup clearImmediate call
		duk_push_c_lightfunc(ctx, [](duk_context* ctx)
		{
			return js_timers__clear_timeout(ctx);
		}, 1, 1, 0);
		duk_dup(ctx, -1);
		duk_dup(ctx, -1);
		duk_put_global_string(ctx, "clearImmediate");
		duk_put_global_string(ctx, "clearTimeout");
		duk_put_global_string(ctx, "clearInterval");
		// store timers context into global stash timers prop
		duk_push_global_stash(ctx);
		duk_push_object(ctx);

		// store timer context pointer into timers object stash.
		const auto timers_ctx = new timers_context();
		duk_push_pointer(ctx, timers_ctx);
		duk_put_prop_string(ctx, -2, JS_TIMERS_CTX_PROP);
		// create callbacks prop, used to store js function callbacks
		duk_push_object(ctx);
		duk_put_prop_string(ctx, -2, JS_TIMERS_CALLBACKS_PROP);

		// setup timers object stash finalizer to help us to free 
		duk_push_c_function(ctx, [](duk_context* ctx)
		{
			duk_push_current_function(ctx);
			duk_get_prop_string(ctx, -1, JS_TIMERS_CTX_PROP);
			if (!duk_is_pointer(ctx, -1))
				return 0;

			const auto timers_ctx = static_cast<timers_context*>(duk_get_pointer(ctx, -1));
			timers_ctx->timers_ = {};
			delete timers_ctx;
			return 0;
		}, 0);
		// also, store timers_ctx pointer into finalizer too.
		duk_push_pointer(ctx, timers_ctx);
		duk_put_prop_string(ctx, -2, JS_TIMERS_CTX_PROP);
		duk_set_finalizer(ctx, -2);

		duk_put_prop_string(ctx, -2, JS_TIMERS_PROP);
		duk_pop(ctx);
	}
	
	void js_timers_exec_pending_timers(duk_context* ctx)
	{
		JS_ASSERT_HEAP(ctx);
		const auto timers = js_timers__get_context(ctx);
		if(!timers)
			return;

		ea::queue<ea::shared_ptr<timeout_desc>> new_queue;
		const auto t_now = duk_get_now(ctx);

		while(!timers->timers_.empty())
		{
			auto timer = timers->timers_.front();
			timers->timers_.pop();

			if(timer->ignore_)
			{
				js_timers__remove_callback(ctx, timer->callback_);
				continue;
			}

			// does timer reaches timeout ?
			// if not, then add back to new queue to process in the next call
			if(timer->timeout_ < t_now)
			{
				new_queue.push(timer);
				continue;
			}

			// if timeout is reached, we must recover
			// callback reference
			duk_push_heapptr(ctx, timer->callback_);
			duk_call(ctx, 0);

			// if timer is interval, put it back at new queue
			// to be re-executed later.
			if (timer->is_interval_)
			{
				timer->timeout_ = t_now + timer->interval_;
				new_queue.push(timer);
				continue;
			}

			// if is not interval. then remove callback from stash
			js_timers__remove_callback(ctx, timer->callback_);
		}

		// overwrite current queue with a new one.
		timers->timers_ = new_queue;
	}

	bool js_timers_has_pending_timers(duk_context* ctx)
	{
		JS_ASSERT_HEAP(ctx);
		const auto t_ctx = js_timers__get_context(ctx);
		if (!t_ctx)
			return false;

		return !t_ctx->timers_.empty();
	}

	void js_timers_clear_timers(duk_context* ctx)
	{
		JS_ASSERT_HEAP(ctx);
		const auto t_ctx = js_timers__get_context(ctx);
		if (!t_ctx)
			return;

		duk_push_global_stash(ctx);
		duk_get_prop_string(ctx, -1, JS_TIMERS_PROP);
		duk_push_object(ctx);
		duk_put_prop_string(ctx, -2, JS_TIMERS_CALLBACKS_PROP);
		duk_pop_2(ctx);

		t_ctx->timers_ = {};
	}
}
