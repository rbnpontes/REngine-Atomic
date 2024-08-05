#include "WebInternalStorage.h"

namespace REngine
{
	namespace Web
	{
		static ea::unordered_map<ea::string, ea::string> s_store = {};

		void internal_storage_set_item(const ea::string& key, const ea::string& value)
		{
			s_store[key] = value;
		}

		ea::string internal_storage_get_item(const ea::string& key)
		{
			const auto it = s_store.find_as(key);
			return it == s_store.end() ? "" : it->second;
		}

		bool internal_storage_has_item(const ea::string& key)
		{
			const auto it = s_store.find_as(key);
			return it != s_store.end();
		}

		void internal_storage_remove_item(const ea::string& key)
		{
			const auto it = s_store.find_as(key);
			s_store.erase(it);
		}

		void internal_storage_clear(bool clear_keys)
		{
			s_store.clear(clear_keys);
		}

	}
}
