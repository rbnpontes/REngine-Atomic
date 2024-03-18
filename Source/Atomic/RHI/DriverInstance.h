#pragma once
#include  "../Graphics/GraphicsDefs.h"

#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/EngineFactory.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>
#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>

namespace REngine
{
    class RENGINE_API DriverInstance
    {
    public:
        DriverInstance();

        Atomic::GraphicsBackend GetBackend() { return backend_; }
        Diligent::IRenderDevice* GetDevice() const { return render_device_; }
        Diligent::IDeviceContext* GetDeviceContext() const { return device_context_; }
        Diligent::ISwapChain* GetSwapChain() const { return swap_chain_; }
    private:
        Atomic::GraphicsBackend backend_;
        
        Diligent::RefCntAutoPtr<Diligent::IEngineFactory> engine_factory_;
        Diligent::RefCntAutoPtr<Diligent::IDeviceContext> device_context_;
        Diligent::RefCntAutoPtr<Diligent::IRenderDevice> render_device_;
        Diligent::RefCntAutoPtr<Diligent::ISwapChain> swap_chain_;
    };
}
