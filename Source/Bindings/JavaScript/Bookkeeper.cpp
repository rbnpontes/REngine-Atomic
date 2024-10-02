#include "Bookkeeper.h"
#include "Utils.h"

#define BOOKKEEPER_PTRS_PROP "bookkeeper_ptrs"
namespace REngine
{
	struct bookkeeper_entry
	{
		void* heap_ptr_;
		void* instance_;
	};

	void bookkeeper__get(duk_context* ctx)
	{
		duk_push_global_stash(ctx);
		if(!duk_get_prop_string(ctx, -1, BOOKKEEPER_PTRS_PROP))
		{
			duk_pop(ctx);
			duk_push_object(ctx);
			duk_dup(ctx, -1);
			duk_put_prop_string(ctx, -3, BOOKKEEPER_PTRS_PROP);
		}

		duk_remove(ctx, -2);
	}

	bookkeeper_entry* bookkeeper__get_entry(duk_context* ctx, void* key)
	{
		assert_heap(ctx);
		bookkeeper__get(ctx);

		duk_push_pointer(ctx, key);
		if(!duk_get_prop(ctx, -2))
		{
			duk_pop_2(ctx);
			return nullptr;
		}

		const auto entry = static_cast<bookkeeper_entry*>(duk_get_pointer(ctx, -1));
		duk_pop_2(ctx);

		return entry;
	}

	void bookkeeper_store(duk_context* ctx, duk_idx_t obj_idx, void* native_obj)
	{
		assert_heap(ctx);

		obj_idx = duk_normalize_index(ctx, obj_idx);
		void* obj_ptr = duk_get_heapptr(ctx, obj_idx);

		bookkeeper__get(ctx);

		const auto bookkeeper_idx = duk_get_top_index(ctx);
		const auto entry = new bookkeeper_entry{ obj_ptr, native_obj };

		// store entry with native_obj and obj_ptr as key
		// [native_obj] = obj
		duk_push_pointer(ctx, native_obj);
		duk_push_pointer(ctx, entry);
		duk_put_prop(ctx, bookkeeper_idx);
		// [obj_ptr]  = obj
		duk_push_pointer(ctx, obj_ptr);
		duk_push_pointer(ctx, entry);
		duk_put_prop(ctx, bookkeeper_idx);

		// pop bookkeeper object
		duk_pop(ctx);
	}

	duk_bool_t bookkeeper_restore(duk_context* ctx, void* ptr)
	{
		if (!ptr)
		{
			duk_push_null(ctx);
			return false;
		}

		const auto entry = bookkeeper__get_entry(ctx, ptr);
		if(!entry)
		{
			duk_push_null(ctx);
			return false;
		}

		duk_push_heapptr(ctx, entry->heap_ptr_);
		return true;
	}

	void bookkeeper_remove(duk_context* ctx, void* ptr)
	{
		assert_heap(ctx);
		const auto entry = bookkeeper__get_entry(ctx, ptr);
		if (!entry)
			return;

		bookkeeper__get(ctx);
		duk_push_pointer(ctx, entry->heap_ptr_);
		duk_del_prop(ctx, -2);
		duk_push_pointer(ctx, entry->instance_);
		duk_del_prop(ctx, -2);
		delete entry;

		duk_pop(ctx);
	}
}
