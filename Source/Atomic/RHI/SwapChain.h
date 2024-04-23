#pragma once
#include "./RHITypes.h"
#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>

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
}