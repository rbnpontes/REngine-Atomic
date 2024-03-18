#include "./DriverInstance.h"

namespace REngine
{
    using namespace Atomic;
    DriverInstance::DriverInstance() :
        backend_(GraphicsBackend::D3D11),
        engine_factory_(nullptr),
        device_context_(nullptr),
        render_device_(nullptr),
        swap_chain_(nullptr)
    {
    }
    
}
