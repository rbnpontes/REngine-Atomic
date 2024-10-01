#pragma once

#include <EASTL/string.h>
#include <EASTL/unordered_map.h>
#include <EASTL/unordered_set.h>
#include <EASTL/vector.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/array.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/shared_array.h>
#include <EASTL/queue.h>
#include <EASTL/functional.h>
#include <EASTL/stack.h>
#include <stdint.h>
#include <typeinfo>

namespace ea = eastl;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

inline void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    return ::operator new(size);
}

inline void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    return ::operator new(size);
}

#define type_name(type) typeid(type).name()