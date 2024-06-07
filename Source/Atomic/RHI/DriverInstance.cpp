#include "./DriverInstance.h"
#include "./DiligentUtils.h"
#include "./PipelineStateBuilder.h"
#include "../IO/Log.h"
#include "../Graphics/ConstantBuffer.h"
#include "../Core/Profiler.h"
#include "../Graphics/Graphics.h"
#include "./SwapChain.h"

#if WIN32
#include <DiligentCore/Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h>
#include <DiligentCore/Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h>
#endif
#include <DiligentCore/Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h>

#include <DiligentCore/Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h>

#include <DiligentCore/Graphics/GraphicsEngineOpenGL/interface/DeviceContextGL.h>
#include <DiligentCore/Graphics/GraphicsEngineOpenGL/interface/SwapChainGL.h>

#include <DiligentCore/Common/interface/ObjectBase.hpp>


namespace REngine
{
    using namespace Atomic;
    static const char* s_backend_names_tbl[] = {
        "GraphicsBackend::D3D11",
        "GraphicsBackend::D3D12",
        "GraphicsBackend::Vulkan",
        "GraphicsBackend::OpenGL",
        ""
    };

    static void fill_create_info(const DriverInstanceInitDesc& init_desc, unsigned adapter_id, Diligent::EngineCreateInfo& ci)
    {
        ci.NumDeferredContexts = init_desc.num_deferred_contexts;
        ci.AdapterId = adapter_id;
#if ATOMIC_DEBUG
        ci.EnableValidation = true;
#endif
    }

    static TextureFormat get_best_depth_format(Diligent::IRenderDevice* device, TextureFormat default_fmt) {
        static const TextureFormat formats[] = {
            TextureFormat::TEX_FORMAT_D24_UNORM_S8_UINT,
            TextureFormat::TEX_FORMAT_D32_FLOAT_S8X24_UINT,
            TextureFormat::TEX_FORMAT_D32_FLOAT,
            TextureFormat::TEX_FORMAT_D16_UNORM,
        };
        
        for(const auto tex_fmt : formats) 
        {
            if(device->GetTextureFormatInfoExt(tex_fmt).BindFlags & Diligent::BIND_DEPTH_STENCIL)
                return tex_fmt;
        }
        
        ATOMIC_LOGWARNING("Not found a suitable depth format. Using default");
        return default_fmt;
    }
    
    DriverInstance::DriverInstance(Atomic::Graphics* graphics) :
        graphics_(graphics),
        backend_(GraphicsBackend::D3D11),
        engine_factory_(nullptr),
        device_contexts_({}),
        render_device_(nullptr),
        swap_chain_(nullptr)
    {
        // Setup default constant buffer sizes for Vertex Shaders
        SetConstantBufferSize(Atomic::VS, Atomic::SP_FRAME, 64/*i can't have a cbuffer too small*/);
        SetConstantBufferSize(Atomic::VS, Atomic::SP_CAMERA, 288/*Shader Default*/);
        SetConstantBufferSize(Atomic::VS, Atomic::SP_ZONE, 96/*Shader Default*/);
        SetConstantBufferSize(Atomic::VS, Atomic::SP_LIGHT, 512/*Shader Default*/);
        SetConstantBufferSize(Atomic::VS, Atomic::SP_MATERIAL, 8192/*Half of 16kb*/);
        SetConstantBufferSize(Atomic::VS, Atomic::SP_OBJECT, 6256 /*Shader Default*/);
        SetConstantBufferSize(Atomic::VS, Atomic::SP_CUSTOM, 64 /*Shader Default*/);
        // Setup default constant buffer sizes for Pixel Shaders
        SetConstantBufferSize(Atomic::PS, Atomic::SP_FRAME, 64/*i can't have a cbuffer too small*/);
        SetConstantBufferSize(Atomic::PS, Atomic::SP_CAMERA, 48/*Shader Default*/);
        SetConstantBufferSize(Atomic::PS, Atomic::SP_ZONE, 80/*Shader Default*/);
        SetConstantBufferSize(Atomic::PS, Atomic::SP_LIGHT, 512/*Shader Default*/);
        SetConstantBufferSize(Atomic::PS, Atomic::SP_MATERIAL, 8192/*Half of 16kb*/);
        SetConstantBufferSize(Atomic::PS, Atomic::SP_OBJECT, 6256 /*Shader Default*/);
        SetConstantBufferSize(Atomic::PS, Atomic::SP_CUSTOM, 64 /*Shader Default*/);

        for (uint32_t i = 0; i < _countof(constant_buffers_); ++i)
            constant_buffers_[i] = {};
    }

    void DriverInstance::Release()
    {
        device_contexts_.Clear();
        swap_chain_ = nullptr;
        render_device_ = nullptr;
        engine_factory_ = nullptr;
    }

    bool DriverInstance::IsInitialized() const
    {
        return render_device_ != nullptr && swap_chain_ != nullptr;
    }

    
    bool DriverInstance::InitDevice(const DriverInstanceInitDesc& init_desc)
    {
        using namespace Diligent;
        if (IsInitialized())
            return true;

        const auto is_opengl = init_desc.backend == GraphicsBackend::OpenGL ||
            init_desc.backend == GraphicsBackend::OpenGLES;

        if(is_opengl && !init_desc.gl_context)
            throw std::runtime_error("OpenGL backend requires GL Context!");

        SwapChainDesc swap_chain_desc;
        swap_chain_desc.Width = init_desc.window_size.x_;
        swap_chain_desc.Height = init_desc.window_size.y_;
        swap_chain_desc.BufferCount = init_desc.triple_buffer ? 3 : 2;
#if RENGINE_PLATFORM_MACOS
        // Apple Environments requires triple buffer
        swap_chain_desc.BufferCount = 3;
#endif
        swap_chain_desc.ColorBufferFormat = init_desc.color_buffer_format;
        swap_chain_desc.DepthBufferFormat = Diligent::TEX_FORMAT_UNKNOWN;

        FullScreenModeDesc fullscreen_mode_desc;
        fullscreen_mode_desc.RefreshRateDenominator = 1;
        fullscreen_mode_desc.RefreshRateNumerator = init_desc.refresh_rate;
        
        auto num_deferred_contexts = init_desc.num_deferred_contexts;
        if(is_opengl)
            num_deferred_contexts = 0;
        device_contexts_.Resize(num_deferred_contexts + 1);
        memset(device_contexts_.Buffer(), 0x0, sizeof(Diligent::IDeviceContext*) * device_contexts_.Size());
        

        auto device_contexts = new IDeviceContext*[num_deferred_contexts + 1];

        ATOMIC_LOGDEBUGF("Graphics Backend: %s", s_backend_names_tbl[static_cast<u8>(init_desc.backend)]);
        switch (init_desc.backend)
        {
#if WIN32
        case GraphicsBackend::D3D11:
            {
                const auto factory = GetEngineFactoryD3D11();
                engine_factory_ = factory;
                
                EngineD3D11CreateInfo ci = {};
#if ATOMIC_DEBUG
                factory->SetMessageCallback(OnDebugMessage);
                ci.SetValidationLevel(VALIDATION_LEVEL_2);
#endif
                ci.GraphicsAPIVersion = {11, 0};
                fill_create_info(init_desc, FindBestAdapter(ci.AdapterId, init_desc.backend), ci);

                factory->CreateDeviceAndContextsD3D11(ci, &render_device_, device_contexts);
        		factory->CreateSwapChainD3D11(render_device_, device_contexts[0], swap_chain_desc, fullscreen_mode_desc, init_desc.window, &swap_chain_);
            }
            break;
        case GraphicsBackend::D3D12:
            {
                const auto factory = GetEngineFactoryD3D12();
                engine_factory_ = factory;
                factory->LoadD3D12();

                EngineD3D12CreateInfo ci = {};
#if ATOMIC_DEBUG
                factory->SetMessageCallback(OnDebugMessage);
#endif
                ci.GraphicsAPIVersion = { 11, 0};
                fill_create_info(init_desc, FindBestAdapter(init_desc.adapter_id, init_desc.backend), ci);

                factory->CreateDeviceAndContextsD3D12(ci, &render_device_, device_contexts);
                factory->CreateSwapChainD3D12(render_device_, device_contexts[0], swap_chain_desc, fullscreen_mode_desc, init_desc.window, &swap_chain_);
            }
            break;
#endif
        case GraphicsBackend::Vulkan:
            {
                const auto factory = GetEngineFactoryVk();
                engine_factory_ = factory;

                EngineVkCreateInfo ci = {};
#if ATOMIC_DEBUG
                factory->SetMessageCallback(OnDebugMessage);
                const char* const ignore_debug_messages[] = {
                    "UNASSIGNED-CoreValidation-Shader-OutputNotConsumed"
                };

                ci.ppIgnoreDebugMessageNames= ignore_debug_messages;
                ci.IgnoreDebugMessageCount = _countof(ignore_debug_messages);
#endif
                fill_create_info(init_desc, FindBestAdapter(init_desc.adapter_id, init_desc.backend), ci);

                factory->CreateDeviceAndContextsVk(ci, &render_device_, device_contexts);
                factory->CreateSwapChainVk(render_device_, device_contexts[0], swap_chain_desc, init_desc.window, &swap_chain_);
            }
            break;
        case GraphicsBackend::OpenGL:
        case GraphicsBackend::OpenGLES:
            {
                const auto factory = GetEngineFactoryOpenGL();
                engine_factory_ = factory;

                EngineGLCreateInfo ci = {};
#if ATOMIC_DEBUG
                factory->SetMessageCallback(OnDebugMessage);
#endif
                fill_create_info(init_desc, FindBestAdapter(init_desc.adapter_id, init_desc.backend), ci);

                Diligent::IDeviceContext* device_context = nullptr;
                ci.GLContext = init_desc.gl_context.get();
                factory->AttachToActiveGLContext(ci, &render_device_, &device_context);
                device_contexts[0] = device_context;
                
                SwapChainOpenGlCreateDesc create_desc;
                create_desc.swap_chain_desc = &swap_chain_desc;
                create_desc.device = render_device_;
                create_desc.device_context = device_context;
                create_desc.window = graphics_->GetSDLWindow();
                
                Diligent::ISwapChainGL* swapchain_gl;
                swapchain_gl = REngine::swapchain_create_opengl(create_desc);
                Diligent::RefCntAutoPtr<Diligent::IDeviceContextGL>(device_context, Diligent::IID_DeviceContextGL)
                    ->SetSwapChain(swapchain_gl);
                
                swap_chain_ = swapchain_gl;
            }
            break;
        default:
            ATOMIC_LOGERRORF("Unsupported Graphics Backend Type %s", s_backend_names_tbl[static_cast<unsigned>(init_desc.backend)]);
            return false;
        }

        for(unsigned i =0; i < device_contexts_.Size(); ++i)
            device_contexts_[i] = device_contexts[i];
        
        delete[] device_contexts;
        backend_ = init_desc.backend;

        // Try init MSAA
        u8 multi_sample = GetSupportedMultiSample(swap_chain_->GetDesc().ColorBufferFormat, init_desc.multisample);
        Diligent::ISwapChain* default_swapchain = swap_chain_;
        
        const auto depth_format = get_best_depth_format(render_device_, init_desc.depth_buffer_format);
        if(!swap_chain_->GetDepthBufferDSV())
        {
            if (multi_sample == 1)
                swapchain_create_wrapper(this, depth_format, &default_swapchain);
            else
                swapchain_create_msaa(this, depth_format, multi_sample, &default_swapchain);
        }
        
        swap_chain_ = default_swapchain;
        multisample_ = multi_sample;
        InitDefaultConstantBuffers();
        return true;
    }

    unsigned DriverInstance::FindBestAdapter(unsigned adapter_id, GraphicsBackend backend) const
    {
        Diligent::Version graphics_version = {};
#if WIN32
        if(backend == GraphicsBackend::D3D11 || backend == GraphicsBackend::D3D12)
            graphics_version = { 11, 0};
#endif
        Vector<Diligent::GraphicsAdapterInfo> adapters;
        unsigned adapters_count = 0;
        engine_factory_->EnumerateAdapters(graphics_version, adapters_count, nullptr);
        adapters.Resize(adapters_count);
        engine_factory_->EnumerateAdapters(graphics_version, adapters_count, adapters.Buffer());

        if(adapter_id >= adapters.Size())
            adapter_id = M_MAX_UNSIGNED;
        
        Diligent::GraphicsAdapterInfo adapter;
        if(adapter_id == M_MAX_UNSIGNED)
        {
            const auto discrete_adapters = adapters.Filter([](auto x) { return x.Type == Diligent::ADAPTER_TYPE_DISCRETE; });
            if(discrete_adapters.Size() == 0)
            {
                adapter = adapters.FirstOrDefault([](auto x) { return x.Type == Diligent::ADAPTER_TYPE_INTEGRATED; });
                if(adapter.Type != Diligent::ADAPTER_TYPE_UNKNOWN)
                    return adapters.IndexOf(adapter);
                adapter = adapters.FirstOrDefault([](auto x) { return x.Type == Diligent::ADAPTER_TYPE_SOFTWARE; });
                if(adapter.Type != Diligent::ADAPTER_TYPE_UNKNOWN)
                    return adapters.IndexOf(adapter);
            }
            
            return 0;
        }

        return adapter_id;
    }

    u8 DriverInstance::GetSupportedMultiSample(Atomic::TextureFormat format, int multi_sample) const
    {
        multi_sample = NextPowerOfTwo(Clamp(multi_sample, 1, 16));

        const auto& format_info = render_device_->GetTextureFormatInfoExt(format);
        while (multi_sample > 1 && ((format_info.SampleCounts & multi_sample) == 0))
            multi_sample >>= 1;
        return Max(1, multi_sample);
    }

    Atomic::SharedPtr<Atomic::ConstantBuffer> DriverInstance::GetConstantBuffer(const ShaderType type, const ShaderParameterGroup group)
    {
        const uint8_t index = GetConstantBufferIndex(type, group);
		if(index >= static_cast<uint8_t>(_countof(constant_buffers_)))
            return {};

        if(!IsInitialized())
            return {};

        auto buffer = constant_buffers_[index];
        if(constant_buffers_[index])
            return buffer;
        buffer = CreateConstantBuffer(type, group, GetConstantBufferSize(type, group));
		constant_buffers_[index] = buffer;

        // Every time we update default constant buffers, we need to update SRB caches too.
        srb_cache_update_default_cbuffers(type, group, buffer->GetGPUObject().Cast<Diligent::IBuffer>(Diligent::IID_Buffer));
        return buffer;
    }

    void DriverInstance::UploadBufferChanges()
    {
        for (const auto& buffer : constant_buffers_)
        {
            ATOMIC_PROFILE(DriverInstance::UploadBufferChanges);
            if (buffer && buffer->IsDirty())
                buffer->Apply();
        }
    }

    void DriverInstance::ClearConstantBuffers()
    {
        for(uint32_t i =0; i < _countof(constant_buffers_); ++i)
			constant_buffers_[i] = nullptr;
    }

    void DriverInstance::MakeBuffersAsDirty()
    {
        for (u32 i = 0; i < _countof(constant_buffers_); ++i)
            if (constant_buffers_[i])
                constant_buffers_[i]->MakeDirty();
    }

    void DriverInstance::InitDefaultConstantBuffers()
    {
        for(uint8_t i =0; i < static_cast<uint8_t>(MAX_SHADER_TYPES); ++i)
        {
	        for(uint8_t j = 0; j < static_cast<uint8_t>(MAX_SHADER_PARAMETER_GROUPS); ++j)
	        {
			    const auto type = static_cast<ShaderType>(i);
			    const auto group = static_cast<ShaderParameterGroup>(j);
                constant_buffers_[GetConstantBufferIndex(type, group)] =
                    CreateConstantBuffer(type, group, GetConstantBufferSize(type, group));
	        }
        }
    }

    Atomic::SharedPtr<Atomic::ConstantBuffer> DriverInstance::CreateConstantBuffer(const Atomic::ShaderType type, const Atomic::ShaderParameterGroup grp,
                                                            const uint32_t size) const
    {
        using namespace Atomic;
        Atomic::String name = "Atomic::ConstantBuffer(";
        name.AppendWithFormat("%s)", utils_get_shader_parameter_group_name(type, grp));

        const auto constant_buffer = new ConstantBuffer(graphics_->GetContext());
        constant_buffer->SetDebugName(name);
        constant_buffer->SetSize(size);

		return SharedPtr<ConstantBuffer>(constant_buffer);
    }

    void DriverInstance::OnDebugMessage(Diligent::DEBUG_MESSAGE_SEVERITY severity, const char* message, const char* function, const char* file, int line)
    {
        Atomic::String log = "[Diligent]: ";
        log = log.AppendWithFormat("\nMessage: %s\nFunction: %s\nFile:%s\nLine: %i",
            message,
            function,
            file,
            line);

        if(severity == Diligent::DEBUG_MESSAGE_SEVERITY_FATAL_ERROR)
            log = Atomic::String("[FATAL]") + log;
        
        switch (severity)
        {
        case Diligent::DEBUG_MESSAGE_SEVERITY_INFO:
            ATOMIC_LOGINFO(log);
            break;
        case Diligent::DEBUG_MESSAGE_SEVERITY_WARNING:
            ATOMIC_LOGWARNING(log);
            break;
        case Diligent::DEBUG_MESSAGE_SEVERITY_ERROR:
        case Diligent::DEBUG_MESSAGE_SEVERITY_FATAL_ERROR:
            ATOMIC_LOGERROR(log);
            break;
        }
    }
}
