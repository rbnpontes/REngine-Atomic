#pragma once
#include "./RHITypes.h"

#if RENGINE_PLATFORM_IOS
    #include <OpenGLES/ES3/gl.h>
#else
    #include <GLEW/glew.h>
#endif

#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <DiligentCore/Graphics/GraphicsEngineOpenGL/interface/SwapChainGL.h>

struct SDL_Window;
namespace REngine
{
	class DriverInstance;
	/// \brief Create MSAA SwapChain. This SwapChain auto resolves internal buffers
	///	when goes to present.
	/// \param driver 
	/// \param depth_fmt 
	/// \param multi_sample 
	/// \param swapchain 
	void swapchain_create_msaa(const DriverInstance* driver, Diligent::TEXTURE_FORMAT depth_fmt, u32 multi_sample, Diligent::ISwapChain** swapchain);
	/// \brief Works in the same way of SwapChain. But generally this method is called when swapchain
	///	doesn't have a depth buffer created. In this case, this method creates an internal buffer
	///	and wraps current swapchain.
	/// \param driver 
	/// \param depth_fmt 
	/// \param swapchain 
	void swapchain_create_wrapper(const DriverInstance* driver, Diligent::TEXTURE_FORMAT depth_fmt, Diligent::ISwapChain** swapchain);
    
    struct SwapChainOpenGlCreateDesc {
        Diligent::SwapChainDesc* swap_chain_desc{nullptr};
        Diligent::IRenderDevice* device{nullptr};
        Diligent::IDeviceContext* device_context{nullptr};
        SDL_Window* window{nullptr};
    };
    Diligent::ISwapChainGL* swapchain_create_opengl(const SwapChainOpenGlCreateDesc& create_desc);
}
