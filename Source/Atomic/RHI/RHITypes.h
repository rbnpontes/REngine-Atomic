#pragma once
#include "../Core/Variant.h"
#include "../Container/Str.h"
#include "../Container/Vector.h"
#include "../Container/ArrayPtr.h"
#include "../Graphics/GraphicsDefs.h"
#include <DiligentCore/Graphics/GraphicsEngine/interface/InputLayout.h>

namespace REngine
{
    struct InputLayoutElementDesc
    {
        unsigned input_index{};
        unsigned buffer_index{};
        unsigned buffer_stride{};
        unsigned element_offset{};
        unsigned instance_step_rate{};
        Atomic::VertexElementType element_type;
    };

    struct InputLayoutDesc
    {
        unsigned num_elements{};
        InputLayoutElementDesc elements[Diligent::MAX_LAYOUT_ELEMENTS]{};
    };
    
    struct PipelineStateOutputDesc
    {
        Diligent::TEXTURE_FORMAT depth_stencil_format{};
        uint8_t num_rts{0};
        Diligent::TEXTURE_FORMAT render_target_formats[Atomic::MAX_RENDERTARGETS]{};
        uint8_t multi_sample{1};
    };

    struct SamplerDesc
    {
        Atomic::TextureFilterMode filter_mode{Atomic::FILTER_DEFAULT};
        uint8_t anisotropy{};
        bool shadow_compare{};
        Atomic::TextureAddressMode address_u{Atomic::ADDRESS_WRAP};
        Atomic::TextureAddressMode address_v{Atomic::ADDRESS_WRAP};
        Atomic::TextureAddressMode address_w{Atomic::ADDRESS_WRAP};

        unsigned ToHash() const
        {
            unsigned hash = filter_mode;
            Atomic::CombineHash(hash, anisotropy);
            Atomic::CombineHash(hash, shadow_compare);
            Atomic::CombineHash(hash, address_u);
            Atomic::CombineHash(hash, address_v);
            Atomic::CombineHash(hash, address_w);
            return hash;
        }
    };
    
    struct ImmutableSamplersDesc
    {
        Atomic::String name{};
        SamplerDesc sampler;

        unsigned ToHash() const
        {
            unsigned hash = Atomic::StringHash::Calculate(name.CString());
            Atomic::CombineHash(hash, sampler.ToHash());
            return hash;
        }
    };

	struct ShaderCompilerDesc
    {
        Atomic::String source_code{};
        Atomic::ShaderType type{};
    };
    struct ShaderCompilerPreProcessResult
    {
        Atomic::String source_code{};
        Atomic::String error{};
        bool has_error{false};
    };

    struct ShaderCompilerResult
    {
        Atomic::PODVector<uint8_t> spirv_code;
        Atomic::String error{};
        bool has_error{ false };
    };

    struct ShaderCompilerConstantBufferDesc
    {
        Atomic::String name{};
        uint32_t size{};
        // Note: if constant/uniform buffer matches to ShaderParameterGroup name, then this value
        // will be defined.
        Atomic::ShaderParameterGroup parameter_group{};
    };

    struct ShaderCompilerReflectDesc
    {
        void* spirv_code{ nullptr };
        uint32_t length{ 0 };
        Atomic::ShaderType type{};
    };

    struct ShaderCompilerReflectInputElement
    {
        Atomic::String name{};
        uint8_t index{};
        Atomic::VertexElementType element_type{};
        Atomic::VertexElementSemantic semantic{};
        uint8_t semantic_index{};
    };

    struct ShaderCompilerReflectInfo
    {
        Atomic::StringVector samplers{};
        bool used_texture_units[Atomic::MAX_TEXTURE_UNITS]{};
        Atomic::HashMap<Atomic::StringHash, ShaderCompilerConstantBufferDesc> constant_buffers{};
        uint32_t constant_buffer_sizes[Atomic::MAX_SHADER_PARAMETER_GROUPS];
        Atomic::HashMap<Atomic::StringHash, Atomic::ShaderParameter> parameters{};

        uint64_t element_hash{};
        Atomic::Vector<ShaderCompilerReflectInputElement> input_elements{};
    };
    struct ShaderCompilerHlslDesc
    {
        void* spirv_code{ nullptr };
        uint32_t length{ 0 };
    };

    struct ShaderCompilerBinDesc
    {
        void* byte_code{ nullptr };
        uint32_t byte_code_size{ 0 };
        Atomic::ShaderType type{};
        Atomic::ShaderByteCodeType byte_code_type{ Atomic::ShaderByteCodeType::Max};
        ShaderCompilerReflectInfo* reflect_info{ nullptr };
    };
    struct ShaderCompilerImportBinResult
    {
        bool has_error{false};
        Atomic::String error{};
        Atomic::ShaderType type{Atomic::MAX_SHADER_TYPES};
        ShaderCompilerReflectInfo reflect_info{};
        Atomic::ShaderByteCodeType byte_code_type{Atomic::ShaderByteCodeType::Max};
        Atomic::SharedArrayPtr<uint8_t> byte_code{nullptr};
        uint32_t byte_code_size{0};
    };
}