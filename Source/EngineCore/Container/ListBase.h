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

#include "../Container/Allocator.h"
#include "../Container/Swap.h"

namespace Atomic
{

/// Doubly-linked list node base class.
struct ListNodeBase
{
    /// Construct.
    ListNodeBase() :
        prev_(0),
        next_(0)
    {
    }

    /// Previous node.
    ListNodeBase* prev_;
    /// Next node.
    ListNodeBase* next_;
};

/// Doubly-linked list iterator base class.
struct ListIteratorBase
{
    /// Construct.
    ListIteratorBase() :
        ptr_(0)
    {
    }

    /// Construct with a node pointer.
    explicit ListIteratorBase(ListNodeBase* ptr) :
        ptr_(ptr)
    {
    }

    /// Test for equality with another iterator.
    bool operator ==(const ListIteratorBase& rhs) const { return ptr_ == rhs.ptr_; }

    /// Test for inequality with another iterator.
    bool operator !=(const ListIteratorBase& rhs) const { return ptr_ != rhs.ptr_; }

    /// Go to the next node.
    void GotoNext()
    {
        if (ptr_)
            ptr_ = ptr_->next_;
    }

    /// Go to the previous node.
    void GotoPrev()
    {
        if (ptr_)
            ptr_ = ptr_->prev_;
    }

    /// Node pointer.
    ListNodeBase* ptr_;
};

/// Doubly-linked list base class.
class ATOMIC_API ListBase
{
public:
    /// Construct.
    ListBase() :
        head_(0),
        tail_(0),
        allocator_(0),
        size_(0)
    {
    }

    /// Swap with another linked list.
    void Swap(ListBase& rhs)
    {
        Atomic::Swap(head_, rhs.head_);
        Atomic::Swap(tail_, rhs.tail_);
        Atomic::Swap(allocator_, rhs.allocator_);
        Atomic::Swap(size_, rhs.size_);
    }

protected:
    /// Head node pointer.
    ListNodeBase* head_;
    /// Tail node pointer.
    ListNodeBase* tail_;
    /// Node allocator.
    AllocatorBlock* allocator_;
    /// Number of nodes.
    unsigned size_;
};

}
