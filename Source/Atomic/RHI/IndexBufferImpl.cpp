#include "Buffer.h"
#include "DriverInstance.h"
#include "../Precompiled.h"

#include "../Graphics/Texture.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/IndexBuffer.h"
#include "../IO/Log.h"

#include "../DebugNew.h"

namespace Atomic
{

void IndexBuffer::OnDeviceLost()
{
    // No-op on Direct3D11
}

void IndexBuffer::OnDeviceReset()
{
    // No-op on Direct3D11
}

void IndexBuffer::Release()
{
    Unlock();
    
    if (graphics_ && graphics_->GetIndexBuffer() == this)
        graphics_->SetIndexBuffer(nullptr);

    object_ = nullptr;
}

bool IndexBuffer::SetData(const void* data)
{
    if (!data)
    {
        ATOMIC_LOGERROR("Null pointer for index buffer data");
        return false;
    }
    
    if (!indexSize_)
    {
        ATOMIC_LOGERROR("Index size not defined, can not set index buffer data");
        return false;
    }

    const auto data_size = indexCount_ * indexSize_;
    if (shadowData_ && data != shadowData_.Get())
        memcpy(shadowData_.Get(), data, data_size);

    if(!object_)
        return true;

    const auto buffer = object_.Cast<Diligent::IBuffer>(Diligent::IID_Buffer);
    if(!dynamic_)
    {
        graphics_->GetImpl()->GetDeviceContext()->UpdateBuffer(buffer,
            0,
            data_size,
            data,
            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        return true;
    }
    
    void* hw_data = MapBuffer(0, indexCount_, true);
    if(!hw_data)
        return false;

    memcpy(hw_data, data, data_size);
    UnmapBuffer();
    
    return true;
}

bool IndexBuffer::SetDataRange(const void* data, unsigned start, unsigned count, bool discard)
{
    if (start == 0 && count == indexCount_)
        return SetData(data);
    
    if (!data)
    {
        ATOMIC_LOGERROR("Null pointer for index buffer data");
        return false;
    }
    
    if (!indexSize_)
    {
        ATOMIC_LOGERROR("Index size not defined, can not set index buffer data");
        return false;
    }
    
    if (start + count > indexCount_)
    {
        ATOMIC_LOGERROR("Illegal range for setting new index buffer data");
        return false;
    }
    
    if (!count)
        return true;
    
    const auto buffer = object_.Cast<Diligent::IBuffer>(Diligent::IID_Buffer);
    const auto data_offset = start * indexSize_;
    const auto data_size = count * indexSize_;
    
    if (shadowData_ && shadowData_.Get() + data_offset != data)
        memcpy(shadowData_.Get() + data_offset, data, data_size);

    if(!object_)
        return true;

    if(!dynamic_)
    {
        graphics_->GetImpl()->GetDeviceContext()->UpdateBuffer(buffer,
            data_offset,
            data_offset + data_size,
            data,
            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        return true;
    }
    
    void* hw_data = MapBuffer(start, count, discard);
    if(!hw_data)
        return false;
    
    memcpy(hw_data, data, data_size);
    UnmapBuffer();
    return true;
}

void* IndexBuffer::Lock(unsigned start, unsigned count, bool discard)
{
    if (lockState_ != LOCK_NONE)
    {
        ATOMIC_LOGERROR("Index buffer already locked");
        return nullptr;
    }
    
    if (!indexSize_)
    {
        ATOMIC_LOGERROR("Index size not defined, can not lock index buffer");
        return nullptr;
    }
    
    if (start + count > indexCount_)
    {
        ATOMIC_LOGERROR("Illegal range for locking index buffer");
        return nullptr;
    }
    
    if (!count)
        return nullptr;
    
    lockStart_ = start;
    lockCount_ = count;
    
    // Because shadow data must be kept in sync, can only lock hardware buffer if not shadowed
    if (object_ && !shadowData_ && dynamic_)
        return MapBuffer(start, count, discard);

    if (shadowData_)
    {
        lockState_ = LOCK_SHADOW;
        return shadowData_.Get() + start * indexSize_;
    }

    if (graphics_)
    {
        lockState_ = LOCK_SCRATCH;
        lockScratchData_ = graphics_->ReserveScratchBuffer(count * indexSize_);
        return lockScratchData_;
    }

    return nullptr;
}

void IndexBuffer::Unlock()
{
    switch (lockState_)
    {
    case LOCK_HARDWARE:
        UnmapBuffer();
        break;
    
    case LOCK_SHADOW:
        SetDataRange(shadowData_.Get() + lockStart_ * indexSize_, lockStart_, lockCount_);
        lockState_ = LOCK_NONE;
        break;
    
    case LOCK_SCRATCH:
        SetDataRange(lockScratchData_, lockStart_, lockCount_);
        if (graphics_)
            graphics_->FreeScratchBuffer(lockScratchData_);
        lockScratchData_ = nullptr;
        lockState_ = LOCK_NONE;
        break;
    
    default: break;
    }
}

bool IndexBuffer::Create()
{
    Release();
    
    if (!indexCount_ || !graphics_)
        return true;

    // TODO: add name support on index buffer
    Diligent::BufferDesc buffer_desc = {};
    buffer_desc.BindFlags = Diligent::BIND_INDEX_BUFFER;
    buffer_desc.CPUAccessFlags = dynamic_ ? Diligent::CPU_ACCESS_WRITE : Diligent::CPU_ACCESS_NONE;
    buffer_desc.Usage = dynamic_ ? Diligent::USAGE_DYNAMIC : Diligent::USAGE_DEFAULT;
    buffer_desc.Size = indexCount_ * indexSize_;

    Diligent::IBuffer* buffer = nullptr;
    graphics_->GetImpl()->GetDevice()->CreateBuffer(buffer_desc, nullptr, &buffer);

    if(!buffer)
    {
        ATOMIC_LOGERROR("Failed to create index buffer");
        return false;
    }

    object_ = buffer;
    return true;
}

bool IndexBuffer::UpdateToGPU()
{
    if (object_ && shadowData_)
        return SetData(shadowData_.Get());
    return false;
}

void* IndexBuffer::MapBuffer(unsigned start, unsigned count, bool discard)
{
    void* hw_data = nullptr;

    if(!object_)
        return hw_data;

    const auto buffer = object_.Cast<Diligent::IBuffer>(Diligent::IID_Buffer);
    graphics_->GetImpl()->GetDeviceContext()->MapBuffer(buffer,
        Diligent::MAP_WRITE,
        discard ? Diligent::MAP_FLAG_DISCARD : Diligent::MAP_FLAG_NONE,
        hw_data);

    if(hw_data)
    {
        lockState_ = LOCK_HARDWARE;
        return hw_data;
    }

    ATOMIC_LOGERROR("Failed to map index buffer");
    return hw_data;
}

void IndexBuffer::UnmapBuffer()
{
    if(!object_ || lockState_ != LOCK_HARDWARE)
        return;

    const auto buffer = object_.Cast<Diligent::IBuffer>(Diligent::IID_Buffer);
    graphics_->GetImpl()->GetDeviceContext()->UnmapBuffer(buffer, Diligent::MAP_WRITE);
    lockState_ = LOCK_NONE;
}

}
