#include "./SwapChain.h"
#include "../IO/Log.h"
#include "./DriverInstance.h"

#include <DiligentCore/Common/interface/DefaultRawMemoryAllocator.hpp>
#include <DiligentCore/Graphics/GraphicsEngineOpenGL/interface/RenderDeviceGL.h>
#include <DiligentCore/Graphics/GraphicsEngine/include/SwapChainBase.hpp>

#include <SDL2/SDL.h>

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

    class SwapChainOpenGl : public Diligent::SwapChainBase<Diligent::ISwapChainGL> {
    public:
        using Base = Diligent::SwapChainBase<Diligent::ISwapChainGL>;
        SwapChainOpenGl(
                        Diligent::IReferenceCounters* ref_counters,
                        const Diligent::SwapChainDesc& swap_chain_desc,
                        Diligent::IRenderDevice* render_device,
                        Diligent::IDeviceContext* device_context,
                        SDL_Window* window)
        : Base(ref_counters, render_device, device_context, swap_chain_desc),
            window_(window)
        {
            InitializeParameters();
            CreateDummyBuffers();
        }
        
        void DILIGENT_CALL_TYPE Present(u32 sync_interval) override {
            SDL_GL_SwapWindow(window_);
        }
        
        void DILIGENT_CALL_TYPE SetFullscreenMode(const Diligent::DisplayModeAttribs& display_mode) override
        {
        }
        void DILIGENT_CALL_TYPE SetWindowedMode() override
        {
        }
        void DILIGENT_CALL_TYPE Resize(u32 new_width, u32 new_height, Diligent::SURFACE_TRANSFORM new_pre_transform) override 
        {
            if(new_pre_transform == Diligent::SURFACE_TRANSFORM_OPTIMAL)
                new_pre_transform = Diligent::SURFACE_TRANSFORM_IDENTITY;
            
            if(new_pre_transform != Diligent::SURFACE_TRANSFORM_IDENTITY)
                throw std::runtime_error("Surface Transform != Identity is not supported on OpenGL.");
            
            if(Base::Resize(new_width, new_height, new_pre_transform))
                CreateDummyBuffers();
        }
        
        GLuint DILIGENT_CALL_TYPE GetDefaultFBO() const override {
            return default_framebuffer_;
        }
        
        Diligent::ITextureView* DILIGENT_CALL_TYPE GetCurrentBackBufferRTV() override {
            return render_target_view_;
        }
        Diligent::ITextureView* DILIGENT_CALL_TYPE GetDepthBufferDSV() override {
            return depth_stencil_view_;
        }
    private:
        bool IsSrgb() const {
            int effective_srgb{};
            if(SDL_GL_GetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, &effective_srgb) != 0)
                return false;
            return effective_srgb != 0;
        }
        Diligent::TEXTURE_FORMAT GetDepthStencilFormat() const {
            static const Diligent::TEXTURE_FORMAT default_format = Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
            
            int effective_depth_bits{};
            if(SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &effective_depth_bits))
                return default_format;
            
            int effective_stencil_bits{};
            if(SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &effective_stencil_bits))
                return default_format;
            
            if(effective_depth_bits == 16 && effective_stencil_bits == 0)
                return Diligent::TEX_FORMAT_D16_UNORM;
            else if(effective_depth_bits == 24 && effective_stencil_bits == 0)
                return Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
            else if(effective_depth_bits == 24 && effective_stencil_bits == 8)
                return Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
            else if(effective_depth_bits == 32 && effective_stencil_bits == 0)
                return Diligent::TEX_FORMAT_D32_FLOAT;
            else if(effective_depth_bits == 32 && effective_stencil_bits == 8)
                return Diligent::TEX_FORMAT_D32_FLOAT_S8X24_UINT;
            return default_format;
        };
        
        void InitializeParameters() {
            Diligent::SwapChainDesc& swap_chain_desc = m_SwapChainDesc;
            
            if(swap_chain_desc.PreTransform == Diligent::SURFACE_TRANSFORM_OPTIMAL)
                swap_chain_desc.PreTransform = Diligent::SURFACE_TRANSFORM_IDENTITY;
            
#if RENGINE_PLATFORM_IOS
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&default_framebuffer_));
#endif
            int width{};
            int height{};
            
            SDL_GL_GetDrawableSize(window_, &width, &height);
            swap_chain_desc.Width = static_cast<u32>(width);
            swap_chain_desc.Height = static_cast<u32>(height);
            swap_chain_desc.ColorBufferFormat = IsSrgb() 
                ? Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB
                : Diligent::TEX_FORMAT_RGBA8_UNORM;
            swap_chain_desc.DepthBufferFormat = GetDepthStencilFormat();
        }
        
        void CreateDummyBuffers() {
            if (m_SwapChainDesc.Width == 0 || m_SwapChainDesc.Height == 0)
                return;
            
            Diligent::RefCntAutoPtr<Diligent::IRenderDeviceGL> device_gl(m_pRenderDevice, Diligent::IID_RenderDeviceGL);
            
            const Diligent::SwapChainDesc& swap_chain_desc = m_SwapChainDesc;
            
            Diligent::TextureDesc dummy_tex_desc;
            dummy_tex_desc.Name = "Main Back buffer";
            dummy_tex_desc.Type = Diligent::RESOURCE_DIM_TEX_2D;
            dummy_tex_desc.Format = swap_chain_desc.ColorBufferFormat;
            dummy_tex_desc.Width = swap_chain_desc.Width;
            dummy_tex_desc.Height = swap_chain_desc.Height;
            dummy_tex_desc.BindFlags = Diligent::BIND_RENDER_TARGET;
            
            Diligent::RefCntAutoPtr<Diligent::ITexture> dummy_rt;
            device_gl->CreateDummyTexture(dummy_tex_desc,
                                          Diligent::RESOURCE_STATE_RENDER_TARGET,
                                          &dummy_rt);
            render_target_view_ = dummy_rt->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
            
            
            dummy_tex_desc.Name = "Main Depth buffer";
            dummy_tex_desc.Format = swap_chain_desc.DepthBufferFormat;
            dummy_tex_desc.BindFlags = Diligent::BIND_DEPTH_STENCIL;
            
            Diligent::RefCntAutoPtr<Diligent::ITexture> dummy_depth;
            device_gl->CreateDummyTexture(dummy_tex_desc,
                                          Diligent::RESOURCE_STATE_DEPTH_WRITE,
                                          &dummy_depth);
            depth_stencil_view_ = dummy_depth->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
        }
        
        SDL_Window* window_;
        
        Diligent::RefCntAutoPtr<Diligent::ITextureView> render_target_view_;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> depth_stencil_view_;
        
        GLuint default_framebuffer_;
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

    Diligent::ISwapChainGL* swapchain_create_opengl(const SwapChainOpenGlCreateDesc& create_desc)
    {
        if(!create_desc.window) 
        {
            ATOMIC_LOGERROR("Can't create SwapChain. Window must not be null");
            return;
        }
        
        if(!create_desc.device)
        {
            ATOMIC_LOGERROR("Can't create SwapChain. RenderDevice must not be null");
            return;
        }
        
        if(!create_desc.device_context){
            ATOMIC_LOGERROR("Can't create SwapChain. Device Context must not be null");
            return;
        }
        
        auto& allocator = Diligent::DefaultRawMemoryAllocator::GetAllocator();
        SwapChainOpenGl* gl_swapchain = NEW_RC_OBJ(allocator, "SwapChainOpenGl instance", SwapChainOpenGl)(
           *create_desc.swap_chain_desc,
           create_desc.device,
           create_desc.device_context,
           create_desc.window
        );
        gl_swapchain->AddRef();
        return gl_swapchain;
    }
}
