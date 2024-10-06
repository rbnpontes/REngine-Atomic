#pragma once
#include "../Container/TypeTraits.h"

#define type_name(type) typeid(type).name()

#if defined(ENGINE_BINDING_TOOL)
	#define ENGINE_BINDING_DEF_CHCK_METHOD static void __engine_object() { }
#else
	#define ENGINE_BINDING_DEF_CHCK_METHOD(...)
#endif

#define ENGINE_OBJECT_DEF_TYPE_ID() \
	virtual type_id GetTypeId() const { return GetTypeIdStatic(); } \
	static type_id GetTypeIdStatic() { \
		static const int anchor = 0; \
		return reinterpret_cast<type_id>(&anchor); \
	} \
	ENGINE_BINDING_DEF_CHCK_METHOD()
#define ENGINE_STRUCT_DEF_TYPE_ID() \
	static type_id GetTypeId() { \
		static const int anchor = 0; \
		return reinterpret_cast<type_id>(&anchor); \
	} \
	ENGINE_BINDING_DEF_CHCK_METHOD()

#define ENGINE_OBJECT() \
	public: \
		ENGINE_OBJECT_DEF_TYPE_ID()
#define ENGINE_STRUCT() ENGINE_STRUCT_DEF_TYPE_ID()