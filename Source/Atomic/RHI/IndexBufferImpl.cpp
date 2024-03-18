#include "../Precompiled.h"

#include "../Core/Context.h"
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
    throw std::exception("Not implemented");
    // Unlock();
    //
    // if (graphics_ && graphics_->GetIndexBuffer() == this)
    //     graphics_->SetIndexBuffer(0);
    //
    // ATOMIC_SAFE_RELEASE(object_.ptr_);
}

bool IndexBuffer::SetData(const void* data)
{
    throw new std::exception("Not implemented");
    // if (!data)
    // {
    //     ATOMIC_LOGERROR("Null pointer for index buffer data");
    //     return false;
    // }
    //
    // if (!indexSize_)
    // {
    //     ATOMIC_LOGERROR("Index size not defined, can not set index buffer data");
    //     return false;
    // }
    //
    // if (shadowData_ && data != shadowData_.Get())
    //     memcpy(shadowData_.Get(), data, indexCount_ * indexSize_);
    //
    // if (object_.ptr_)
    // {
    //     if (dynamic_)
    //     {
    //         void* hwData = MapBuffer(0, indexCount_, true);
    //         if (hwData)
    //         {
    //             memcpy(hwData, data, indexCount_ * indexSize_);
    //             UnmapBuffer();
    //         }
    //         else
    //             return false;
    //     }
    //     else
    //     {
    //         D3D11_BOX destBox;
    //         destBox.left = 0;
    //         destBox.right = indexCount_ * indexSize_;
    //         destBox.top = 0;
    //         destBox.bottom = 1;
    //         destBox.front = 0;
    //         destBox.back = 1;
    //
    //         graphics_->GetImpl()->GetDeviceContext()->UpdateSubresource((ID3D11Buffer*)object_.ptr_, 0, &destBox, data, 0, 0);
    //     }
    // }
    //
    // return true;
}

bool IndexBuffer::SetDataRange(const void* data, unsigned start, unsigned count, bool discard)
{
    throw new std::exception("Not implemented");
    // if (start == 0 && count == indexCount_)
    //     return SetData(data);
    //
    // if (!data)
    // {
    //     ATOMIC_LOGERROR("Null pointer for index buffer data");
    //     return false;
    // }
    //
    // if (!indexSize_)
    // {
    //     ATOMIC_LOGERROR("Index size not defined, can not set index buffer data");
    //     return false;
    // }
    //
    // if (start + count > indexCount_)
    // {
    //     ATOMIC_LOGERROR("Illegal range for setting new index buffer data");
    //     return false;
    // }
    //
    // if (!count)
    //     return true;
    //
    // if (shadowData_ && shadowData_.Get() + start * indexSize_ != data)
    //     memcpy(shadowData_.Get() + start * indexSize_, data, count * indexSize_);
    //
    // if (object_.ptr_)
    // {
    //     if (dynamic_)
    //     {
    //         void* hwData = MapBuffer(start, count, discard);
    //         if (hwData)
    //         {
    //             memcpy(hwData, data, count * indexSize_);
    //             UnmapBuffer();
    //         }
    //         else
    //             return false;
    //     }
    //     else
    //     {
    //         D3D11_BOX destBox;
    //         destBox.left = start * indexSize_;
    //         destBox.right = destBox.left + count * indexSize_;
    //         destBox.top = 0;
    //         destBox.bottom = 1;
    //         destBox.front = 0;
    //         destBox.back = 1;
    //
    //         graphics_->GetImpl()->GetDeviceContext()->UpdateSubresource((ID3D11Buffer*)object_.ptr_, 0, &destBox, data, 0, 0);
    //     }
    // }
    //
    // return true;
}

void* IndexBuffer::Lock(unsigned start, unsigned count, bool discard)
{
    throw new std::exception("Not implemented");
    // if (lockState_ != LOCK_NONE)
    // {
    //     ATOMIC_LOGERROR("Index buffer already locked");
    //     return 0;
    // }
    //
    // if (!indexSize_)
    // {
    //     ATOMIC_LOGERROR("Index size not defined, can not lock index buffer");
    //     return 0;
    // }
    //
    // if (start + count > indexCount_)
    // {
    //     ATOMIC_LOGERROR("Illegal range for locking index buffer");
    //     return 0;
    // }
    //
    // if (!count)
    //     return 0;
    //
    // lockStart_ = start;
    // lockCount_ = count;
    //
    // // Because shadow data must be kept in sync, can only lock hardware buffer if not shadowed
    // if (object_.ptr_ && !shadowData_ && dynamic_)
    //     return MapBuffer(start, count, discard);
    // else if (shadowData_)
    // {
    //     lockState_ = LOCK_SHADOW;
    //     return shadowData_.Get() + start * indexSize_;
    // }
    // else if (graphics_)
    // {
    //     lockState_ = LOCK_SCRATCH;
    //     lockScratchData_ = graphics_->ReserveScratchBuffer(count * indexSize_);
    //     return lockScratchData_;
    // }
    // else
    //     return 0;
}

void IndexBuffer::Unlock()
{
    throw new std::exception("Not implemented");
    // switch (lockState_)
    // {
    // case LOCK_HARDWARE:
    //     UnmapBuffer();
    //     break;
    //
    // case LOCK_SHADOW:
    //     SetDataRange(shadowData_.Get() + lockStart_ * indexSize_, lockStart_, lockCount_);
    //     lockState_ = LOCK_NONE;
    //     break;
    //
    // case LOCK_SCRATCH:
    //     SetDataRange(lockScratchData_, lockStart_, lockCount_);
    //     if (graphics_)
    //         graphics_->FreeScratchBuffer(lockScratchData_);
    //     lockScratchData_ = 0;
    //     lockState_ = LOCK_NONE;
    //     break;
    //
    // default: break;
    // }
}

bool IndexBuffer::Create()
{
    throw new std::exception("Not implemented");
    // Release();
    //
    // if (!indexCount_)
    //     return true;
    //
    // if (graphics_)
    // {
    //     D3D11_BUFFER_DESC bufferDesc;
    //     memset(&bufferDesc, 0, sizeof bufferDesc);
    //     bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    //     bufferDesc.CPUAccessFlags = dynamic_ ? D3D11_CPU_ACCESS_WRITE : 0;
    //     bufferDesc.Usage = dynamic_ ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    //     bufferDesc.ByteWidth = (UINT)(indexCount_ * indexSize_);
    //
    //     HRESULT hr = graphics_->GetImpl()->GetDevice()->CreateBuffer(&bufferDesc, 0, (ID3D11Buffer**)&object_.ptr_);
    //     if (FAILED(hr))
    //     {
    //         ATOMIC_SAFE_RELEASE(object_.ptr_);
    //         ATOMIC_LOGD3DERROR("Failed to create index buffer", hr);
    //         return false;
    //     }
    // }
    //
    // return true;
}

bool IndexBuffer::UpdateToGPU()
{
    throw new std::exception("Not implemented");
    // if (object_.ptr_ && shadowData_)
    //     return SetData(shadowData_.Get());
    // else
    //     return false;
}

void* IndexBuffer::MapBuffer(unsigned start, unsigned count, bool discard)
{
    throw new std::exception("Not implemented");
    // void* hwData = 0;
    //
    // if (object_.ptr_)
    // {
    //     D3D11_MAPPED_SUBRESOURCE mappedData;
    //     mappedData.pData = 0;
    //
    //     HRESULT hr = graphics_->GetImpl()->GetDeviceContext()->Map((ID3D11Buffer*)object_.ptr_, 0, discard ? D3D11_MAP_WRITE_DISCARD :
    //         D3D11_MAP_WRITE, 0, &mappedData);
    //     if (FAILED(hr) || !mappedData.pData)
    //         ATOMIC_LOGD3DERROR("Failed to map index buffer", hr);
    //     else
    //     {
    //         hwData = mappedData.pData;
    //         lockState_ = LOCK_HARDWARE;
    //     }
    // }
    //
    // return hwData;
}

void IndexBuffer::UnmapBuffer()
{
    throw new std::exception("Not implemented");
    // if (object_.ptr_ && lockState_ == LOCK_HARDWARE)
    // {
    //     graphics_->GetImpl()->GetDeviceContext()->Unmap((ID3D11Buffer*)object_.ptr_, 0);
    //     lockState_ = LOCK_NONE;
    // }
}

}
