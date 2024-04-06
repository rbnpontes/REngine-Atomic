#include "../Graphics/ConstantBuffer.h"

#include  "./DriverInstance.h"
#include "../Graphics/Graphics.h"
#include "../IO/Log.h"

#include <DiligentCore/Graphics/GraphicsEngine/interface/Buffer.h>


namespace Atomic
{
    void ConstantBuffer::OnDeviceReset()
    {
        // Diligent handles this for us
    }

    void ConstantBuffer::Release()
    {
        object_ = nullptr;
        shadowData_.Reset();
        size_ = 0;
    }

    bool ConstantBuffer::SetSize(unsigned size)
    {
        Release();
        if(!size)
        {
            ATOMIC_LOGERROR("Can not create zero-sized constant buffer");
            return false;
        }

        // Round up to next 16 bytes
        size += 15;
        size &= 0xfffffff0;

        size_ = size;
        dirty_ = false;
        shadowData_ = new uint8_t[size_];
        memset(shadowData_.Get(), 0x0, size_);

        if (!graphics_)
            return true;

        using namespace Diligent;
        BufferDesc buffer_desc = {};
        buffer_desc.Name = dbg_name_.CString();
        buffer_desc.Usage = USAGE_DYNAMIC;
        buffer_desc.BindFlags = BIND_UNIFORM_BUFFER;
        buffer_desc.CPUAccessFlags = CPU_ACCESS_WRITE;
        buffer_desc.Size = size_;

        IBuffer* buffer = nullptr;
        graphics_->GetImpl()->GetDevice()->CreateBuffer(buffer_desc, nullptr, &buffer);

        if(!buffer)
        {
   			ATOMIC_LOGERROR("Failed to create constant buffer");
			return false;
		}

        object_ = buffer;
        return true;
    }

    void ConstantBuffer::Apply()
    {
        if(!dirty_ || !object_)
			return;

        using namespace Diligent;
        auto buffer = object_.Cast<IBuffer>(IID_Buffer);
        void* mapped_data = nullptr;
        graphics_->GetImpl()->GetDeviceContext()->MapBuffer(buffer, MAP_WRITE, MAP_FLAG_DISCARD, mapped_data);

        if (!mapped_data)
            return;

        memcpy(mapped_data, shadowData_.Get(), size_);
        graphics_->GetImpl()->GetDeviceContext()->UnmapBuffer(buffer, MAP_WRITE);
        dirty_ = false;
    }
}
