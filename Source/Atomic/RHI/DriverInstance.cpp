#include "./DriverInstance.h"
#include "./DiligentUtils.h"
#include "./PipelineStateBuilder.h"
#include "../IO/Log.h"
#include "../Graphics/ConstantBuffer.h"

#if WIN32
#include <DiligentCore/Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h>
#include <DiligentCore/Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h>
#endif
#include <DiligentCore/Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h>
#include <DiligentCore/Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h>

#include <DiligentCore/Common/interface/ObjectBase.hpp>

#include "Graphics/Graphics.h"

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
        SetConstantBufferSize(Atomic::VS, Atomic::SP_LIGHT, 304/*Shader Default*/);
        SetConstantBufferSize(Atomic::VS, Atomic::SP_MATERIAL, 8192/*Half of 16kb*/);
        SetConstantBufferSize(Atomic::VS, Atomic::SP_OBJECT, 6256 /*Shader Default*/);
        SetConstantBufferSize(Atomic::VS, Atomic::SP_CUSTOM, 64 /*Shader Default*/);
        // Setup default constant buffer sizes for Pixel Shaders
        SetConstantBufferSize(Atomic::PS, Atomic::SP_FRAME, 64/*i can't have a cbuffer too small*/);
        SetConstantBufferSize(Atomic::PS, Atomic::SP_CAMERA, 48/*Shader Default*/);
        SetConstantBufferSize(Atomic::PS, Atomic::SP_ZONE, 80/*Shader Default*/);
        SetConstantBufferSize(Atomic::PS, Atomic::SP_LIGHT, 328/*Shader Default*/);
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

        SwapChainDesc swap_chain_desc;
        swap_chain_desc.Width = init_desc.window_size.x_;
        swap_chain_desc.Height = init_desc.window_size.y_;
        swap_chain_desc.ColorBufferFormat = init_desc.color_buffer_format;
        swap_chain_desc.DepthBufferFormat = init_desc.depth_buffer_format;

        auto num_deferred_contexts = init_desc.num_deferred_contexts;
        if(init_desc.backend == GraphicsBackend::OpenGL)
            num_deferred_contexts = 0;
        device_contexts_.Resize(num_deferred_contexts + 1);
        memset(device_contexts_.Buffer(), 0x0, sizeof(Diligent::IDeviceContext*) * device_contexts_.Size());
        

        auto device_contexts = new IDeviceContext*[num_deferred_contexts + 1];
        
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
                factory->CreateSwapChainD3D11(render_device_, device_contexts[0], swap_chain_desc, {}, init_desc.window, &swap_chain_);
            }
            break;
        case GraphicsBackend::D3D12:
            {
                const auto factory = GetEngineFactoryD3D12();
                engine_factory_ = factory;

                EngineD3D12CreateInfo ci = {};
#if ATOMIC_DEBUG
                factory->SetMessageCallback(OnDebugMessage);
#endif
                ci.GraphicsAPIVersion = { 11, 0};
                fill_create_info(init_desc, FindBestAdapter(init_desc.adapter_id, init_desc.backend), ci);

                factory->LoadD3D12();
                factory->CreateDeviceAndContextsD3D12(ci, &render_device_, device_contexts);
                factory->CreateSwapChainD3D12(render_device_, device_contexts[0], swap_chain_desc, {}, init_desc.window, &swap_chain_);
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
            {
                const auto factory = GetEngineFactoryOpenGL();
                engine_factory_ = factory;

                EngineGLCreateInfo ci = {};
#if ATOMIC_DEBUG
                factory->SetMessageCallback(OnDebugMessage);
#endif
                ci.Window = init_desc.window;
                fill_create_info(init_desc, FindBestAdapter(init_desc.adapter_id, init_desc.backend), ci);

                IDeviceContext* device_context = nullptr;
                factory->CreateDeviceAndSwapChainGL(ci, &render_device_, &device_context, swap_chain_desc, &swap_chain_);
                device_contexts[0] = device_context;
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

    PODVector<int> DriverInstance::GetMultiSampleLevels(Diligent::TEXTURE_FORMAT color_fmt, Diligent::TEXTURE_FORMAT depth_fmt) const
    {
        PODVector<int> levels;
        const auto& color_fmt_info = render_device_->GetTextureFormatInfoExt(color_fmt);
        const auto& depth_fmt_info = render_device_->GetTextureFormatInfoExt(depth_fmt);

        const auto sample_counts = color_fmt_info.SampleCounts & depth_fmt_info.SampleCounts;
        if(sample_counts & Diligent::SAMPLE_COUNT_64)
            levels.Push(64);
        if(sample_counts & Diligent::SAMPLE_COUNT_32)
            levels.Push(32);
        if(sample_counts & Diligent::SAMPLE_COUNT_16)
            levels.Push(16);
        if(sample_counts & Diligent::SAMPLE_COUNT_8)
            levels.Push(8);
        if(sample_counts & Diligent::SAMPLE_COUNT_4)
            levels.Push(4);
        if(sample_counts & Diligent::SAMPLE_COUNT_2)
            levels.Push(2);
        if(sample_counts & Diligent::SAMPLE_COUNT_1)
            levels.Push(1);
        return levels;
    }

    bool DriverInstance::CheckMultiSampleSupport(unsigned multisample, Diligent::TEXTURE_FORMAT color_fmt, Diligent::TEXTURE_FORMAT depth_fmt) const
    {
        const auto& values = GetMultiSampleLevels(color_fmt, depth_fmt);
        return values.Contains(multisample);
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
        for (auto buffer : constant_buffers_)
            if (buffer && buffer->IsDirty())
                buffer->Apply();
    }

    void DriverInstance::ClearConstantBuffers()
    {
        for(uint32_t i =0; i < _countof(constant_buffers_); ++i)
			constant_buffers_[i] = nullptr;
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
