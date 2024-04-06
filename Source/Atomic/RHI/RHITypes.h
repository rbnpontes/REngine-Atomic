#pragma once
#include "../Core/Variant.h"
#include "../Container/Str.h"
#include "../Container/Vector.h"
#include "../Container/ArrayPtr.h"
#include "../Graphics/GraphicsDefs.h"
#include "../Container/Hash.h"
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
        Atomic::VertexElementType element_type{Atomic::MAX_VERTEX_ELEMENT_TYPES};

        uint32_t ToHash() const
        {
	        uint32_t hash = input_index;
            Atomic::CombineHash(hash, buffer_index);
            Atomic::CombineHash(hash, buffer_stride);
            Atomic::CombineHash(hash, element_offset);
            Atomic::CombineHash(hash, instance_step_rate);
            Atomic::CombineHash(hash, element_type);
            return hash;
        }
    };

    struct InputLayoutDesc
    {
        unsigned num_elements{};
        InputLayoutElementDesc elements[Diligent::MAX_LAYOUT_ELEMENTS]{};

        uint32_t ToHash() const
        {
	        uint32_t hash = num_elements;
			for(unsigned i =0; i < num_elements; ++i)
				Atomic::CombineHash(hash, elements[i].ToHash());
			return hash;
		}
    };
    
    struct PipelineStateOutputDesc
    {
        Diligent::TEXTURE_FORMAT depth_stencil_format{};
        uint8_t num_rts{0};
        Diligent::TEXTURE_FORMAT render_target_formats[Atomic::MAX_RENDERTARGETS]{};
        uint8_t multi_sample{1};

        uint32_t ToHash() const
        {
	        uint32_t hash = depth_stencil_format;
			Atomic::CombineHash(hash, num_rts);
			for(unsigned i = 0; i < num_rts; ++i)
				Atomic::CombineHash(hash, render_target_formats[i]);
			Atomic::CombineHash(hash, multi_sample);
			return hash;
        }
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

        uint32_t ToHash() const
        {
	        uint32_t hash = Atomic::StringHash::Calculate(source_code.CString());
			Atomic::CombineHash(hash, type);
			return hash;
		}
    };

	struct ShaderCompilerPreProcessResult
    {
        Atomic::String source_code{};
        Atomic::String error{};
        bool has_error{false};

        uint32_t ToHash() const
        {
	        uint32_t hash = Atomic::StringHash::Calculate(source_code.CString());
            Atomic::CombineHash(hash, Atomic::StringHash::Calculate(error.CString()));
            Atomic::CombineHash(hash, has_error ? 1u : 0u);
            return hash;
        }
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
        uint32_t shader_hash{ 0 };
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
        uint32_t shader_hash{ 0 };
    };
}