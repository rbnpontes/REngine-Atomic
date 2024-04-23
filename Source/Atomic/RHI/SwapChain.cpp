#include "./SwapChain.h"
#include "../IO/Log.h"

#include <DiligentCore/Common/interface/ObjectBase.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <DiligentCore/Common/interface/DefaultRawMemoryAllocator.hpp>

#include "DriverInstance.h"


namespace REngine
{
	class SwapChainWrapper : public Diligent::ObjectBase<Diligent::ISwapChain>
	{
	public:
		using Base = Diligent::ObjectBase<Diligent::ISwapChain>;
		SwapChainWrapper(
			Diligent::IReferenceCounters* ref_counters,
			Diligent::IRenderDevice* device,
			Diligent::IDeviceContext* immediate_context,
			Diligent::ISwapChain* owner,
			Diligent::TEXTURE_FORMAT depth_fmt
		) : Base(ref_counters),
			device_(device),
			context_(immediate_context),
			owner_(owner),
			depth_fmt_(depth_fmt)
		{
		}

		void Init()
		{
			UpdateBuffers();
			UpdateDesc();
		}

		const Diligent::SwapChainDesc& DILIGENT_CALL_TYPE GetDesc() const override { return swap_chain_desc_; }

		void DILIGENT_CALL_TYPE Present(u32 sync_interval) override
		{
			owner_->Present(sync_interval);
		}

		void DILIGENT_CALL_TYPE Resize(Diligent::Uint32 NewWidth, Diligent::Uint32 NewHeight, Diligent::SURFACE_TRANSFORM NewTransform) override
		{
			const auto& swap_chain_desc = owner_->GetDesc();
			const auto old_width = swap_chain_desc.Width;
			const auto old_height = swap_chain_desc.Height;

			owner_->Resize(NewWidth, NewHeight, NewTransform);
			UpdateDesc();

			// Re-Create Buffers size changes
			if (old_width != swap_chain_desc.Width || swap_chain_desc.Height != old_height)
				UpdateBuffers();
		}

		void DILIGENT_CALL_TYPE SetFullscreenMode(const Diligent::DisplayModeAttribs& DisplayMode) override
		{
		}

		void DILIGENT_CALL_TYPE SetWindowedMode() override
		{
		}

		void DILIGENT_CALL_TYPE SetMaximumFrameLatency(Diligent::Uint32 MaxLatency) override
		{
			owner_->SetMaximumFrameLatency(MaxLatency);
		}

		Diligent::ITextureView* DILIGENT_CALL_TYPE GetCurrentBackBufferRTV() override
		{
			return owner_->GetCurrentBackBufferRTV();
		}

		Diligent::ITextureView* DILIGENT_CALL_TYPE GetDepthBufferDSV() override
		{
			return depth_buffer_view_;
		}
	protected:
		virtual void UpdateBuffers()
		{
			CreateDepthBuffer();
		}

		virtual void CreateDepthBuffer()
		{
			Diligent::TextureDesc desc = {};
			GetDepthBufferDesc(desc);

			Diligent::RefCntAutoPtr<Diligent::ITexture> depth_buffer;
			device_->CreateTexture(desc, nullptr, &depth_buffer);

			depth_buffer_view_ = depth_buffer->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
		}
		virtual void GetDepthBufferDesc(Diligent::TextureDesc& desc)
		{
			const auto& swap_chain_desc = owner_->GetDesc();
			desc.Name = "Default Depth Buffer";
			desc.Type = Diligent::RESOURCE_DIM_TEX_2D;
			desc.Width = swap_chain_desc.Width;
			desc.Height = swap_chain_desc.Height;
			desc.Format = depth_fmt_;
			desc.Usage = Diligent::USAGE_DEFAULT;
			desc.BindFlags = Diligent::BIND_DEPTH_STENCIL;
		}
		virtual void UpdateDesc()
		{
			swap_chain_desc_ = owner_->GetDesc();
			swap_chain_desc_.DepthBufferFormat = depth_fmt_;
		}
		Diligent::IRenderDevice* device_;
		Diligent::IDeviceContext* context_;
		Diligent::RefCntAutoPtr<Diligent::ISwapChain> owner_;
		Diligent::TEXTURE_FORMAT depth_fmt_;
		Diligent::RefCntAutoPtr<Diligent::ITextureView> depth_buffer_view_;
		Diligent::SwapChainDesc swap_chain_desc_;
	};

	class SwapChainMsaa : public SwapChainWrapper
	{
	public:

		SwapChainMsaa(Diligent::IReferenceCounters* ref_counters, 
			Diligent::IRenderDevice* device,
			Diligent::IDeviceContext* immediate_context,
			Diligent::ISwapChain* owner,
			Diligent::TEXTURE_FORMAT depth_fmt,
			u32 multi_sample) :
			SwapChainWrapper(ref_counters, device, immediate_context, owner, depth_fmt),
			multi_sample_(multi_sample)
		{
		}

		void DILIGENT_CALL_TYPE Present(u32 sync_interval) override
		{
			// Resolve MSAA render target first before present.
			const auto target = owner_->GetCurrentBackBufferRTV()->GetTexture();
			Diligent::ResolveTextureSubresourceAttribs resolve_attribs;
			resolve_attribs.SrcTextureTransitionMode =
			resolve_attribs.DstTextureTransitionMode =
				Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

			context_->ResolveTextureSubresource(render_target_, target, resolve_attribs);
			SwapChainWrapper::Present(sync_interval);
		}

		Diligent::ITextureView* DILIGENT_CALL_TYPE GetCurrentBackBufferRTV() override
		{
			return render_target_view_;
		}
	protected:
		void UpdateBuffers() override
		{
			CreateRenderTarget();
			SwapChainWrapper::UpdateBuffers();
		}
		void GetDepthBufferDesc(Diligent::TextureDesc& desc) override
		{
			SwapChainWrapper::GetDepthBufferDesc(desc);
			desc.SampleCount = multi_sample_;
			desc.Name = "MSAA Depth Buffer";
		}
	private:
		void CreateRenderTarget()
		{
			const auto& swap_chain_desc = owner_->GetDesc();

			Diligent::TextureDesc desc;
			desc.Name = "MSAA Render Target";
			desc.Type = Diligent::RESOURCE_DIM_TEX_2D;
			desc.Width = swap_chain_desc.Width;
			desc.Height = swap_chain_desc.Height;
			desc.Format = swap_chain_desc.ColorBufferFormat;
			desc.SampleCount = multi_sample_;
			desc.Usage = Diligent::USAGE_DEFAULT;
			desc.BindFlags = Diligent::BIND_RENDER_TARGET;

			device_->CreateTexture(desc, nullptr, &render_target_);
			render_target_view_ = render_target_->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
		}

		u32 multi_sample_;

		Diligent::RefCntAutoPtr<Diligent::ITexture> render_target_;
		Diligent::RefCntAutoPtr<Diligent::ITextureView> render_target_view_;

	};

	void swapchain_create_msaa(const DriverInstance* driver, Diligent::TEXTURE_FORMAT depth_fmt, u32 multi_sample, Diligent::ISwapChain** swapchain)
	{
		if(multi_sample <= 1)
		{
			ATOMIC_LOGWARNING("Can't create MSAA SwapChain. Multi sample level is 1, disabling MSAA.");
			swapchain_create_wrapper(driver, depth_fmt, swapchain);
			return;
		}

		if(!driver)
		{
			ATOMIC_LOGERROR("Can't create MSAA SwapChain. Driver must not be null");
			return;
		}

		if(!swapchain)
		{
			ATOMIC_LOGERROR("Can't create MSAA SwapChain. SwapChain array must not be null.");
			return;
		}

		if(!*swapchain)
		{
			ATOMIC_LOGERROR("Can't create MSAA SwapChain. SwapChain item on array must not be null.");
			return;
		}

		auto& allocator = Diligent::DefaultRawMemoryAllocator::GetAllocator();
		SwapChainMsaa* msaa_swapchain = NEW_RC_OBJ(allocator, "SwapChainWrapper instance", SwapChainMsaa)(
			driver->GetDevice(),
			driver->GetDeviceContext(),
			*swapchain,
			depth_fmt,
			multi_sample
		);
		msaa_swapchain->Init();
		*swapchain = msaa_swapchain;
	}

	void swapchain_create_wrapper(const DriverInstance* driver, Diligent::TEXTURE_FORMAT depth_fmt, Diligent::ISwapChain** swapchain)
	{
		if(!driver)
		{
			ATOMIC_LOGERROR("Can't create SwapChain. Driver must not be null");
			return;
		}

		if(!swapchain)
		{
			ATOMIC_LOGERROR("Can't create SwapChain. SwapChain array must not be null.");
			return;
		}

		if(!*swapchain)
		{
			ATOMIC_LOGERROR("Can't create SwapChain. SwapChain item on array must not be null.");
			return;
		}

		auto& allocator = Diligent::DefaultRawMemoryAllocator::GetAllocator();
		SwapChainWrapper* wrapper = NEW_RC_OBJ(allocator, "SwapChainWrapper instance", SwapChainWrapper)(
			driver->GetDevice(),
			driver->GetDeviceContext(),
			*swapchain,
			depth_fmt
		);
		wrapper->Init();
		*swapchain = wrapper;
	}

}
