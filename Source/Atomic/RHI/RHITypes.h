#pragma once
#include "../Core/Variant.h"
#include "../Container/Str.h"
#include "../Container/Vector.h"
#include "../Graphics/GraphicsDefs.h"
#include "../Container/Hash.h"

#include <DiligentCore/Graphics/GraphicsEngine/interface/TextureView.h>
#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>

#include "InputLayout.h"

namespace Atomic
{
	class ShaderVariation;
    class Texture;
}

namespace REngine
{
    class DriverInstance;

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
        ea::array<InputLayoutElementDesc, Diligent::MAX_LAYOUT_ELEMENTS> elements;

        InputLayoutDesc()
        {
            num_elements = 0;
            elements.fill({});
        }

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
        Atomic::TextureFormat depth_stencil_format{};
        uint8_t num_rts{0};
        ea::array<Atomic::TextureFormat, Atomic::MAX_RENDERTARGETS> render_target_formats{};
        uint8_t multi_sample{1};

        PipelineStateOutputDesc()
        {
            depth_stencil_format = Atomic::TextureFormat::TEX_FORMAT_UNKNOWN;
            num_rts = 0;
            multi_sample = 1;
            render_target_formats.fill(Atomic::TextureFormat::TEX_FORMAT_UNKNOWN);
        }

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
        const char* name;
        Atomic::StringHash name_hash;
        SamplerDesc sampler;

        ImmutableSamplersDesc()
        {
            name = nullptr;
            name_hash = Atomic::StringHash::ZERO;
            sampler = {};
        }

        u32 ToHash() const
        {
            u32 hash = name_hash.Value();
            Atomic::CombineHash(hash, sampler.ToHash());
            return hash;
        }
    };

    struct ShaderResourceTextureDesc
    {
        const char* name{nullptr};
        Atomic::StringHash name_hash{Atomic::StringHash::ZERO};
        Atomic::TextureUnit unit{ Atomic::MAX_TEXTURE_UNITS };
        Diligent::RefCntAutoPtr<Diligent::ITextureView> texture {};
        ea::shared_ptr<Atomic::Texture> owner{};
    };
    typedef ea::array<ShaderResourceTextureDesc, Atomic::MAX_TEXTURE_UNITS> ShaderResourceTextures;

    struct PipelineStateInfo
    {
        Atomic::String debug_name{};

        // BEGIN BLEND STATE
        bool color_write_enabled{true};
        Atomic::BlendMode blend_mode{Atomic::BLEND_REPLACE};
        bool alpha_to_coverage_enabled{false};
        // END BLEND_STATE

        // BEGIN RASTERIZER STATE
        Atomic::FillMode fill_mode{Atomic::FILL_SOLID};
        Atomic::CullMode cull_mode{Atomic::CULL_CCW};
        float constant_depth_bias{0.f};
        float slope_scaled_depth_bias{0.f};
        bool scissor_test_enabled{false};
        bool line_anti_alias{false};
        // END RASTERIZER STATE

        // BEGIN DEPTH STENCIL STATE
        bool depth_write_enabled{true};
        bool stencil_test_enabled{false};
        Atomic::CompareMode depth_cmp_function{Atomic::CMP_LESSEQUAL};
        Atomic::CompareMode stencil_cmp_function{Atomic::CMP_ALWAYS};
        Atomic::StencilOp stencil_op_on_passed{Atomic::OP_KEEP};
        Atomic::StencilOp stencil_op_on_stencil_failed{Atomic::OP_KEEP};
        Atomic::StencilOp stencil_op_depth_failed{Atomic::OP_KEEP};
        u8 stencil_cmp_mask{255};
        u8 stencil_write_mask{255};
        // END DEPTH STENCIL STATE

        InputLayoutDesc input_layout;
        Atomic::PrimitiveType primitive_type{Atomic::TRIANGLE_LIST};
        PipelineStateOutputDesc output;

        u8 num_samplers{0};
        ea::array<ImmutableSamplersDesc, Atomic::MAX_IMMUTABLE_SAMPLERS> immutable_samplers{};

        bool read_only_depth{false};

        Atomic::ShaderVariation* vs_shader{};
        Atomic::ShaderVariation* ps_shader{};
        Atomic::ShaderVariation* ds_shader{};
        Atomic::ShaderVariation* hs_shader{};
        Atomic::ShaderVariation* gs_shader{};

        uint32_t ToHash() const;
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
        ea::vector<u8> spirv_code;
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
        ea::vector<ShaderCompilerReflectInputElement> input_elements{};
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
        ea::shared_array<u8> byte_code{nullptr};
        uint32_t byte_code_size{0};
        uint32_t shader_hash{ 0 };
    };
}