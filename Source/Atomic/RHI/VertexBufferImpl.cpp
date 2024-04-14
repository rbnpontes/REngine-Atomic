#include "DriverInstance.h"
#include "../Precompiled.h"

#include "../Graphics/Graphics.h"
#include "../Graphics/VertexBuffer.h"
#include "../IO/Log.h"

#include "../DebugNew.h"

namespace Atomic
{

void VertexBuffer::OnDeviceLost()
{
    // No-op on Direct3D11
}

void VertexBuffer::OnDeviceReset()
{
    // No-op on Direct3D11
}

void VertexBuffer::Release()
{
    Unlock();
    
    if (graphics_)
    {
        for (unsigned i = 0; i < MAX_VERTEX_STREAMS; ++i)
        {
            if (graphics_->GetVertexBuffer(i) == this)
                graphics_->SetVertexBuffer(nullptr);
        }
    }

    object_ = nullptr;
}

bool VertexBuffer::SetData(const void* data)
{
    if (!data)
    {
        ATOMIC_LOGERROR("Null pointer for vertex buffer data");
        return false;
    }
    
    if (!vertexSize_)
    {
        ATOMIC_LOGERROR("Vertex elements not defined, can not set vertex buffer data");
        return false;
    }
    
    const auto copy_size = vertexSize_ * vertexCount_;
    
    if (shadowData_ && data != shadowData_.Get())
        memcpy(shadowData_.Get(), data, copy_size);
    
    if (!object_)
        return true;

    const auto buffer = object_.Cast<Diligent::IBuffer>(Diligent::IID_Buffer);
    if(!dynamic_)
    {
        graphics_->GetImpl()->GetDeviceContext()->UpdateBuffer(buffer,
            0,
            copy_size,
            data,
            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        return true;
    }

    void* hw_data = MapBuffer(0, vertexCount_, true);
    if (!hw_data)
        return false;
    
    memcpy(hw_data, data, copy_size);
    UnmapBuffer();
    return true;
}

bool VertexBuffer::SetDataRange(const void* data, unsigned start, unsigned count, bool discard)
{
    if (start == 0 && count == vertexCount_)
        return SetData(data);
    
    if (!data)
    {
        ATOMIC_LOGERROR("Null pointer for vertex buffer data");
        return false;
    }
    
    if (!vertexSize_)
    {
        ATOMIC_LOGERROR("Vertex elements not defined, can not set vertex buffer data");
        return false;
    }
    
    if (start + count > vertexCount_)
    {
        ATOMIC_LOGERROR("Illegal range for setting new vertex buffer data");
        return false;
    }

    const auto data_size = count * vertexSize_;
    const auto data_offset = start * vertexSize_;
    if (!count)
        return true;
    
    if (shadowData_ && shadowData_.Get() + data_offset != data)
        memcpy(shadowData_.Get() + data_offset, data, data_size);

    if(!object_)
        return true;

    const auto buffer = object_.Cast<Diligent::IBuffer>(Diligent::IID_Buffer);
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

void* VertexBuffer::Lock(unsigned start, unsigned count, bool discard)
{
    if (lockState_ != LOCK_NONE)
    {
        ATOMIC_LOGERROR("Vertex buffer already locked");
        return nullptr;
    }
    
    if (!vertexSize_)
    {
        ATOMIC_LOGERROR("Vertex elements not defined, can not lock vertex buffer");
        return nullptr;
    }
    
    if (start + count > vertexCount_)
    {
        ATOMIC_LOGERROR("Illegal range for locking vertex buffer");
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
        return shadowData_.Get() + start * vertexSize_;
    }

    if (graphics_)
    {
        lockState_ = LOCK_SCRATCH;
        lockScratchData_ = graphics_->ReserveScratchBuffer(count * vertexSize_);
        return lockScratchData_;
    }

    return nullptr;
}

void VertexBuffer::Unlock()
{
    switch (lockState_)
    {
    case LOCK_HARDWARE:
        UnmapBuffer();
        break;
    
    case LOCK_SHADOW:
        SetDataRange(shadowData_.Get() + lockStart_ * vertexSize_, lockStart_, lockCount_);
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

bool VertexBuffer::Create()
{
    Release();
    
    if (!vertexCount_ || !elementMask_)
        return true;
    
    if (!graphics_)
        return true;

    using namespace Diligent;
    // TODO: add vertex buffer name support
    BufferDesc buffer_desc = {};
    buffer_desc.Name = debug_name_.c_str();
    buffer_desc.BindFlags = BIND_VERTEX_BUFFER;
    buffer_desc.Mode = BUFFER_MODE_UNDEFINED;
    buffer_desc.CPUAccessFlags = dynamic_ ? CPU_ACCESS_WRITE : CPU_ACCESS_NONE;
    buffer_desc.Usage = dynamic_ ? USAGE_DYNAMIC : USAGE_DEFAULT;
    buffer_desc.Size = vertexCount_ * vertexSize_;

    Diligent::IBuffer* buffer = nullptr;
    graphics_->GetImpl()->GetDevice()->CreateBuffer(buffer_desc, nullptr, &buffer);

    if(!buffer)
    {
        ATOMIC_LOGERROR("Failed to create vertex buffer");
        return false;
    }

    object_ = buffer;
    return true;
}

bool VertexBuffer::UpdateToGPU()
{
    if (object_ && shadowData_)
        return SetData(shadowData_.Get());
    return false;
}

void* VertexBuffer::MapBuffer(unsigned start, unsigned count, bool discard)
{
    void* hw_data = nullptr;
    if(!object_)
        return hw_data;

    if(lockState_ == LOCK_HARDWARE)
    {
	    ATOMIC_LOGWARNING("Vertex buffer already locked in hardware mode");
		return gpu_map_ptr_;
	}

    using namespace Diligent;
    const auto buffer = object_.Cast<IBuffer>(IID_Buffer);
    graphics_->GetImpl()->GetDeviceContext()->MapBuffer(buffer,
        MAP_WRITE,
        discard ? MAP_FLAG_DISCARD : MAP_FLAG_NONE, hw_data);

    if(!hw_data)
        ATOMIC_LOGERROR("Failed to map vertex buffer");
    else
        lockState_ = LOCK_HARDWARE;
    
    return gpu_map_ptr_ = hw_data;
}

void VertexBuffer::UnmapBuffer()
{
    if(!object_ || lockState_ != LOCK_HARDWARE)
        return;

    using namespace Diligent;
    const auto buffer = object_.Cast<Diligent::IBuffer>(IID_Buffer);

    graphics_->GetImpl()->GetDeviceContext()->UnmapBuffer(buffer, MAP_WRITE);
    lockState_ = LOCK_NONE;
    gpu_map_ptr_ = nullptr;
}

}
