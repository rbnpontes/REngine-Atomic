#pragma once
#include "../Core/Variant.h"
#include "../Container/Str.h"
#include "../Container/Vector.h"
#include "../Container/ArrayPtr.h"
#include "../Graphics/GraphicsDefs.h"

namespace REngine
{
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