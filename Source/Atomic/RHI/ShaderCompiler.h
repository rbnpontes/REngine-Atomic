#pragma once
#include "./PipelineStateBuilder.h"
#include "./RHITypes.h"

namespace REngine
{
    void shader_compiler_preprocess(const ShaderCompilerDesc& desc, ShaderCompilerPreProcessResult& output);
    void shader_compiler_compile(const ShaderCompilerDesc& desc, const bool optimize, ShaderCompilerResult& output);
    void shader_compiler_reflect(const ShaderCompilerReflectDesc& desc, ShaderCompilerReflectInfo& output);
	void shader_compiler_to_hlsl(const ShaderCompilerHlslDesc& desc, Atomic::String& source_code);
    ea::shared_array<u8> shader_compiler_to_bin(const ShaderCompilerBinDesc& desc, u32* output_length);
    void shader_compiler_import_bin(void* data, u32 data_size, ShaderCompilerImportBinResult& result);
    void shader_compiler_get_file_ext(Atomic::ShaderType type, Atomic::String& ext);
}
