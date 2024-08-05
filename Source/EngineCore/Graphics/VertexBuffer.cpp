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

// This file contains VertexBuffer code common to all graphics APIs.

#include "../Precompiled.h"

#include "../Graphics/Graphics.h"
#include "../Graphics/VertexBuffer.h"
#include "../Math/MathDefs.h"

#include "../DebugNew.h"
#include "Core/CoreEvents.h"
#include "RHI/DriverInstance.h"

namespace Atomic
{

VertexBuffer::VertexBuffer(Context* context, bool forceHeadless) :
    Object(context),
    GPUObject(forceHeadless ? (Graphics*)0 : GetSubsystem<Graphics>()),
    vertexCount_(0),
    elementMask_(0),
    lockState_(LOCK_NONE),
    lockStart_(0),
    lockCount_(0),
    lockScratchData_(0),
    shadowed_(false),
    dynamic_(false),
    discardLock_(false),
    gpu_map_ptr_(nullptr)
{
    UpdateOffsets();

    // Force shadowing mode if graphics subsystem does not exist
    if (!graphics_)
        shadowed_ = true;

    SubscribeToEvent(E_BEGINFRAME, ATOMIC_HANDLER(VertexBuffer, HandleBeginFrame));
}

VertexBuffer::~VertexBuffer()
{
    Release();
}

void VertexBuffer::SetShadowed(bool enable)
{
    // If no graphics subsystem, can not disable shadowing
    if (!graphics_)
        enable = true;

    if (enable != shadowed_)
    {
        if (enable && vertexSize_ && vertexCount_)
            shadowData_ = new unsigned char[vertexCount_ * vertexSize_];
        else
            shadowData_.Reset();

        shadowed_ = enable;
    }
}

bool VertexBuffer::SetSize(unsigned vertexCount, unsigned elementMask, bool dynamic)
{
    return SetSize(vertexCount, GetElements(elementMask), dynamic);
}

bool VertexBuffer::SetSize(unsigned vertexCount, const ea::vector<VertexElement>& elements, bool dynamic)
{
    Unlock();

    vertexCount_ = vertexCount;
    elements_ = elements;
    dynamic_ = dynamic;

    UpdateOffsets();

    if (shadowed_ && vertexCount_ && vertexSize_)
        shadowData_ = new unsigned char[vertexCount_ * vertexSize_];
    else
        shadowData_.Reset();

    return Create();
}

void VertexBuffer::UpdateOffsets()
{
    unsigned elementOffset = 0;
    elementHash_ = 0;
    elementMask_ = 0;

    for(auto& element : elements_)
    {
        element.offset_ = elementOffset;
        elementOffset += ELEMENT_TYPESIZES[element.type_];
        elementHash_ <<= 6;
        elementHash_ += (((int)element.type_ + 1) * ((int)element.semantic_ + 1) + element.index_);

        for (unsigned j = 0; j < MAX_LEGACY_VERTEX_ELEMENTS; ++j)
        {
            const VertexElement& legacy = LEGACY_VERTEXELEMENTS[j];
            if (element.type_ == legacy.type_ && element.semantic_ == legacy.semantic_ && element.index_ == legacy.index_)
                elementMask_ |= (1 << j);
        }
    }

    vertexSize_ = elementOffset;
}

const VertexElement* VertexBuffer::GetElement(VertexElementSemantic semantic, unsigned char index) const
{
    for(const auto& element : elements_)
    {
        if (element.semantic_ == semantic && element.index_ == index)
            return &element;
    }

    return nullptr;
}

const VertexElement* VertexBuffer::GetElement(VertexElementType type, VertexElementSemantic semantic, unsigned char index) const
{
    for(const auto& element : elements_)
    {
        if (element.type_ == type && element.semantic_ == semantic && element.index_ == index)
            return &element;
    }

    return nullptr;
}

const VertexElement* VertexBuffer::GetElement(const ea::vector<VertexElement>& elements, VertexElementType type, VertexElementSemantic semantic, unsigned char index)
{
    for(const auto& element : elements)
    {
        if (element.type_ == type && element.semantic_ == semantic && element.index_ == index)
            return &element;
    }

    return nullptr;
}

bool VertexBuffer::HasElement(const ea::vector<VertexElement>& elements, VertexElementType type, VertexElementSemantic semantic, unsigned char index)
{
    return GetElement(elements, type, semantic, index) != nullptr;
}

u32 VertexBuffer::GetElementOffset(const ea::vector<VertexElement>& elements, VertexElementType type, VertexElementSemantic semantic, unsigned char index)
{
    const VertexElement* element = GetElement(elements, type, semantic, index);
    return element ? element->offset_ : M_MAX_UNSIGNED;
}

ea::vector<VertexElement> VertexBuffer::GetElements(u32 elementMask)
{
    ea::vector<VertexElement> ret;

    for (unsigned i = 0; i < MAX_LEGACY_VERTEX_ELEMENTS; ++i)
    {
        if (elementMask & (1 << i))
            ret.push_back(LEGACY_VERTEXELEMENTS[i]);
    }

    return ret;
}

u32 VertexBuffer::GetVertexSize(const ea::vector<VertexElement>& elements)
{
    u32 size = 0;

    for (const auto element : elements)
	    size += ELEMENT_TYPESIZES[element.type_];

    return size;
}

u32 VertexBuffer::GetVertexSize(u32 elementMask)
{
    u32 size = 0;

    for (u32 i = 0; i < MAX_LEGACY_VERTEX_ELEMENTS; ++i)
    {
        if (elementMask & (1 << i))
            size += ELEMENT_TYPESIZES[LEGACY_VERTEXELEMENTS[i].type_];
    }

    return size;
}

void VertexBuffer::UpdateOffsets(ea::vector<VertexElement>& elements)
{
    u32 element_offset = 0;
    for(auto& element : elements)
    {
        element.offset_ = element_offset;
        element_offset += ELEMENT_TYPESIZES[element.type_];
    }
}

void VertexBuffer::HandleBeginFrame(StringHash eventType, VariantMap& eventData)
{
    if(!dynamic_)
        return;
    const auto backend = graphics_->GetImpl()->GetBackend();
    if (backend == GraphicsBackend::D3D12 || backend == GraphicsBackend::Vulkan)
        dataLost_ = true;
}

}
