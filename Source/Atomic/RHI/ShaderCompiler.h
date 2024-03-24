#pragma once
#include "../Container/Str.h"
#include "../Graphics/GraphicsDefs.h"
#include "Core/Variant.h"

namespace REngine
{
    struct ShaderCompilerPreProcessDesc
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

    void shader_compiler_preprocess(const ShaderCompilerPreProcessDesc& desc, ShaderCompilerPreProcessResult& output);
}
