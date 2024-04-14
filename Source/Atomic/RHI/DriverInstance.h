#pragma once
#include "../Graphics/GraphicsDefs.h"
#include "../Math/Vector2.h"
#include "../Container/Vector.h"
#include "../Container/Ptr.h"
#include "../Graphics/ConstantBuffer.h"

#include <DiligentCore/Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/EngineFactory.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h>
#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>

namespace Atomic
{
    class Graphics;
}
namespace REngine
{
    struct DriverInstanceInitDesc
    {
        Atomic::GraphicsBackend backend{Atomic::GraphicsBackend::D3D12};
        Diligent::NativeWindow window{};
        Atomic::IntVector2 window_size{Atomic::IntVector2::ZERO};
        uint8_t multisample{0};
        uint8_t num_deferred_contexts{0};
        unsigned adapter_id{Atomic::M_MAX_UNSIGNED};

        Diligent::TEXTURE_FORMAT color_buffer_format{};
        Diligent::TEXTURE_FORMAT depth_buffer_format{};
    };

    class RENGINE_API DriverInstance
    {
    public:
        DriverInstance(Atomic::Graphics* graphics);
        void Release();
        bool IsInitialized() const;
        bool InitDevice(const DriverInstanceInitDesc& init_desc);

        Atomic::GraphicsBackend GetBackend() { return backend_; }
        Diligent::IRenderDevice* GetDevice() const { return render_device_; }
        Diligent::IDeviceContext* GetDeviceContext() const { return device_contexts_.At(0); }
        Diligent::ISwapChain* GetSwapChain() const { return swap_chain_; }

        Atomic::PODVector<int> GetMultiSampleLevels(Diligent::TEXTURE_FORMAT color_fmt, Diligent::TEXTURE_FORMAT depth_fmt) const;
        bool CheckMultiSampleSupport(unsigned multisample, Diligent::TEXTURE_FORMAT color_fmt, Diligent::TEXTURE_FORMAT depth_fmt) const;
        uint8_t GetMultiSample() const { return multisample_; }

        uint32_t GetConstantBufferSize(Atomic::ShaderType type, Atomic::ShaderParameterGroup group) const
        {
            const uint8_t index = GetConstantBufferIndex(type, group);
            if(index >= static_cast<uint8_t>(_countof(constant_buffer_sizes_)))
				return 0;
	        return constant_buffer_sizes_[index];
        }
        void SetConstantBufferSize(Atomic::ShaderType type, Atomic::ShaderParameterGroup group, uint32_t size)
        {
	        const uint8_t index = GetConstantBufferIndex(type, group);
            if(index >= static_cast<uint8_t>(_countof(constant_buffer_sizes_)))
				return;
	        constant_buffer_sizes_[index] = size;
            constant_buffers_[index] = nullptr;
        }

        Atomic::SharedPtr<Atomic::ConstantBuffer> GetConstantBuffer(Atomic::ShaderType type, Atomic::ShaderParameterGroup group);
        void UploadBufferChanges();
        void ClearConstantBuffers();
        void MakeBuffersAsDirty();
    private:
        void InitDefaultConstantBuffers();
        Atomic::SharedPtr<Atomic::ConstantBuffer> CreateConstantBuffer(Atomic::ShaderType type, Atomic::ShaderParameterGroup grp, uint32_t size) const;
        static void OnDebugMessage(Diligent::DEBUG_MESSAGE_SEVERITY severity,
            const char* message,
            const char* function,
            const char* file,
            int line);
        unsigned FindBestAdapter(unsigned adapter_id, Atomic::GraphicsBackend backend) const;
        static uint8_t GetConstantBufferIndex(Atomic::ShaderType type, Atomic::ShaderParameterGroup group)
        {
	        return static_cast<uint8_t>(type) * static_cast<uint8_t>(Atomic::MAX_SHADER_PARAMETER_GROUPS) + static_cast<uint8_t>(group);
		}

        Atomic::Graphics* graphics_;
        Atomic::GraphicsBackend backend_;
        
        Diligent::RefCntAutoPtr<Diligent::IEngineFactory> engine_factory_;
        Atomic::PODVector<Diligent::RefCntAutoPtr<Diligent::IDeviceContext>> device_contexts_;
        Diligent::RefCntAutoPtr<Diligent::IRenderDevice> render_device_;
        Diligent::RefCntAutoPtr<Diligent::ISwapChain> swap_chain_;

        uint8_t multisample_;
        uint32_t constant_buffer_sizes_[static_cast<uint8_t>(Atomic::MAX_SHADER_PARAMETER_GROUPS) * static_cast<uint8_t>(Atomic::MAX_SHADER_TYPES)];
        Atomic::SharedPtr<Atomic::ConstantBuffer> constant_buffers_[static_cast<uint8_t>(Atomic::MAX_SHADER_PARAMETER_GROUPS) * static_cast<uint8_t>(Atomic::MAX_SHADER_TYPES)];
    };
}
