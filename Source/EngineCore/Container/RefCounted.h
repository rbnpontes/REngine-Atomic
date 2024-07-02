//
// Copyright (c) 2008-2017 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "Atomic/Atomic.h"
#include "Vector.h"

// ATOMIC BEGIN

#include "../Container/Str.h"
#include "../Math/StringHash.h"

// ATOMIC END

namespace Atomic
{

// ATOMIC BEGIN

/// Instantation type, native code, JS, or C#
enum InstantiationType
{
    INSTANTIATION_NATIVE = 0,
    INSTANTIATION_JAVASCRIPT = 1,
    INSTANTIATION_NET = 2
};

class RefCounted;

// function that is called when ref count goes to 1 or 2+, used for script object lifetime
typedef void (*RefCountChangedFunction)(RefCounted*, int refCount);

// function callback for when a RefCounted is created
typedef void(*RefCountedCreatedFunction)(RefCounted*);

// function callback for when a RefCounted is deleted
typedef void(*RefCountedDeletedFunction)(RefCounted*);

typedef const void* ClassID;

/// Macro to be included in RefCounted derived classes for efficient RTTI
#define ATOMIC_REFCOUNTED(typeName) \
    public: \
        virtual Atomic::ClassID GetClassID() const { return GetClassIDStatic(); } \
        static Atomic::ClassID GetClassIDStatic() { static const int typeID = 0; return (Atomic::ClassID) &typeID; } \
        virtual const Atomic::String& GetTypeName() const { return GetTypeNameStatic(); } \
        static const Atomic::String& GetTypeNameStatic() { static const Atomic::String _typeName(#typeName); return _typeName; }

// ATOMIC END

/// Reference count structure.
struct RefCount
{
    /// Construct.
    RefCount() :
        refs_(0),
        weakRefs_(0)
    {
    }

    /// Destruct.
    ~RefCount()
    {
        // Set reference counts below zero to fire asserts if this object is still accessed
        refs_ = -1;
        weakRefs_ = -1;
    }

    /// Reference count. If below zero, the object has been destroyed.
    int refs_;
    /// Weak reference count.
    int weakRefs_;
};

/// Base class for intrusively reference-counted objects. These are noncopyable and non-assignable.
class ATOMIC_API RefCounted
{
public:
    /// Construct. Allocate the reference count structure and set an initial self weak reference.
    RefCounted();
    /// Destruct. Mark as expired and also delete the reference count structure if no outside weak references exist.
    virtual ~RefCounted();

    /// Increment reference count. Can also be called outside of a SharedPtr for traditional reference counting.
    void AddRef();
    /// Decrement reference count and delete self if no more references. Can also be called outside of a SharedPtr for traditional reference counting.
    void ReleaseRef();
    /// Return reference count.
    int Refs() const;
    /// Return weak reference count.
    int WeakRefs() const;

    /// Return pointer to the reference count structure.
    RefCount* RefCountPtr() { return refCount_; }

// ATOMIC BEGIN

    virtual bool IsObject() const { return false; }

    virtual const String& GetTypeName() const = 0;

    /// Increment reference count. Do not call any lifetime bookkeeping
    void AddRefSilent();

    /// Decrement reference count, do not call any lifetime bookkeeping, don't delete at refcount == 0
    void ReleaseRefSilent();

    virtual ClassID GetClassID() const  = 0;
    static ClassID GetClassIDStatic() { static const int typeID = 0; return (ClassID) &typeID; }

    /// JavaScript VM, heap object which can be pushed directly on stack without any lookups
    inline void* JSGetHeapPtr() const { return jsHeapPtr_; }
    inline void  JSSetHeapPtr(void* heapptr) { jsHeapPtr_ = heapptr; }

    inline InstantiationType GetInstantiationType()  const { return instantiationType_; }
    inline void SetInstantiationType(InstantiationType type) { instantiationType_ = type; }

    static void AddRefCountChangedFunction(RefCountChangedFunction function);
    static void RemoveRefCountChangedFunction(RefCountChangedFunction function);

    static void AddRefCountedCreatedFunction(RefCountedCreatedFunction function);
    static void RemoveRefCountedCreatedFunction(RefCountedCreatedFunction function);

    static void AddRefCountedDeletedFunction(RefCountedDeletedFunction function);
    static void RemoveRefCountedDeletedFunction(RefCountedDeletedFunction function);

// ATOMIC END

private:
    /// Prevent copy construction.
    RefCounted(const RefCounted& rhs);
    /// Prevent assignment.
    RefCounted& operator =(const RefCounted& rhs);

    /// Pointer to the reference count structure.
    RefCount* refCount_;

    // ATOMIC BEGIN

    InstantiationType instantiationType_;
    void* jsHeapPtr_;

    static PODVector<RefCountChangedFunction> refCountChangedFunctions_;
    static PODVector<RefCountedCreatedFunction> refCountedCreatedFunctions_;
    static PODVector<RefCountedDeletedFunction> refCountedDeletedFunctions_;

    // ATOMIC END

};

}
