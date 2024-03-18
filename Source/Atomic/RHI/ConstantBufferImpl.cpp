#include "../Precompiled.h"

#include "../Graphics/Graphics.h"
#include "../Graphics/ConstantBuffer.h"
#include "../IO/Log.h"

#include "../DebugNew.h"

namespace Atomic
{
    void ConstantBuffer::OnDeviceReset()
    {
        // Diligent handles this for us
    }

    void ConstantBuffer::Release()
    {
        throw std::exception("Not Implemented");
    }

    bool ConstantBuffer::SetSize(unsigned size)
    {
        Release();
        throw std::exception("Not implemented");
    }

    void ConstantBuffer::Apply()
    {
        throw std::exception("Not implemented");
    }
}
