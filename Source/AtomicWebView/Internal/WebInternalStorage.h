#pragma once
#include <EngineCore/Container/TypeTraits.h>

namespace REngine
{
	namespace Web
	{
		/**
		 * \brief Set item at memory internal storage
		 * \param key key of item
		 * \param value value of item
		 */
		void internal_storage_set_item(const ea::string& key, const ea::string& value);
		/**
		 * \brief Get item from memory internal storage
		 * \param key Key of item
		 * \return stored item value
		 */
		ea::string internal_storage_get_item(const ea::string& key);
		/**
		 * \brief test if internal memory storage has a given key
		 * \param key Key of item
		 * \return 
		 */
		bool internal_storage_has_item(const ea::string& key);
		/**
		 * \brief Remove item from internal memory storage.
		 * \param key 
		 */
		void internal_storage_remove_item(const ea::string& key);
		/**
		 * \brief clear all stored items.
		 */
		void internal_storage_clear(bool clear_keys = false);
	}
}
