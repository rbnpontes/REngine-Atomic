#include "./ShaderCompiler.h"
#include  <glslang/Public/ShaderLang.h>

#include "Engine/Application.h"
#include "IO/Log.h"


namespace REngine
{
    static bool s_is_initialized_glslang = false;
    static EShLanguage get_stage_type(Atomic::ShaderType type)
    {
        switch (type) {
        case Atomic::VS:
            return EShLangVertex;
        case Atomic::PS:
            return EShLangFragment;
        }

        ATOMIC_LOGERROR("Invalid Shader Type");
        return EShLangCount;
    }

    static TBuiltInResource init_resources()
    {
        TBuiltInResource resources = {};
        resources.maxLights                                 = 32;
        resources.maxClipPlanes                             = 6;
        resources.maxTextureUnits                           = 32;
        resources.maxTextureCoords                          = 32;
        resources.maxVertexAttribs                          = 64;
        resources.maxVertexUniformComponents                = 4096;
        resources.maxVaryingFloats                          = 64;
        resources.maxVertexTextureImageUnits                = 32;
        resources.maxCombinedTextureImageUnits              = 80;
        resources.maxTextureImageUnits                      = 32;
        resources.maxFragmentUniformComponents              = 4096;
        resources.maxDrawBuffers                            = 32;
        resources.maxVertexUniformVectors                   = 128;
        resources.maxVaryingVectors                         = 8;
        resources.maxFragmentUniformVectors                 = 16;
        resources.maxVertexOutputVectors                    = 16;
        resources.maxFragmentInputVectors                   = 15;
        resources.minProgramTexelOffset                     = -8;
        resources.maxProgramTexelOffset                     = 7;
        resources.maxClipDistances                          = 8;
        resources.maxComputeWorkGroupCountX                 = 65535;
        resources.maxComputeWorkGroupCountY                 = 65535;
        resources.maxComputeWorkGroupCountZ                 = 65535;
        resources.maxComputeWorkGroupSizeX                  = 1024;
        resources.maxComputeWorkGroupSizeY                  = 1024;
        resources.maxComputeWorkGroupSizeZ                  = 64;
        resources.maxComputeUniformComponents               = 1024;
        resources.maxComputeTextureImageUnits               = 16;
        resources.maxComputeImageUniforms                   = 8;
        resources.maxComputeAtomicCounters                  = 8;
        resources.maxComputeAtomicCounterBuffers            = 1;
        resources.maxVaryingComponents                      = 60;
        resources.maxVertexOutputComponents                 = 64;
        resources.maxGeometryInputComponents                = 64;
        resources.maxGeometryOutputComponents               = 128;
        resources.maxFragmentInputComponents                = 128;
        resources.maxImageUnits                             = 8;
        resources.maxCombinedImageUnitsAndFragmentOutputs   = 8;
        resources.maxCombinedShaderOutputResources          = 8;
        resources.maxImageSamples                           = 0;
        resources.maxVertexImageUniforms                    = 0;
        resources.maxTessControlImageUniforms               = 0;
        resources.maxTessEvaluationImageUniforms            = 0;
        resources.maxGeometryImageUniforms                  = 0;
        resources.maxFragmentImageUniforms                  = 8;
        resources.maxCombinedImageUniforms                  = 8;
        resources.maxGeometryTextureImageUnits              = 16;
        resources.maxGeometryOutputVertices                 = 256;
        resources.maxGeometryTotalOutputComponents          = 1024;
        resources.maxGeometryUniformComponents              = 1024;
        resources.maxGeometryVaryingComponents              = 64;
        resources.maxTessControlInputComponents             = 128;
        resources.maxTessControlOutputComponents            = 128;
        resources.maxTessControlTextureImageUnits           = 16;
        resources.maxTessControlUniformComponents           = 1024;
        resources.maxTessControlTotalOutputComponents       = 4096;
        resources.maxTessEvaluationInputComponents          = 128;
        resources.maxTessEvaluationOutputComponents         = 128;
        resources.maxTessEvaluationTextureImageUnits        = 16;
        resources.maxTessEvaluationUniformComponents        = 1024;
        resources.maxTessPatchComponents                    = 120;
        resources.maxPatchVertices                          = 32;
        resources.maxTessGenLevel                           = 64;
        resources.maxViewports                              = 16;
        resources.maxVertexAtomicCounters                   = 0;
        resources.maxTessControlAtomicCounters              = 0;
        resources.maxTessEvaluationAtomicCounters           = 0;
        resources.maxGeometryAtomicCounters                 = 0;
        resources.maxFragmentAtomicCounters                 = 8;
        resources.maxCombinedAtomicCounters                 = 8;
        resources.maxAtomicCounterBindings                  = 1;
        resources.maxVertexAtomicCounterBuffers             = 0;
        resources.maxTessControlAtomicCounterBuffers        = 0;
        resources.maxTessEvaluationAtomicCounterBuffers     = 0;
        resources.maxGeometryAtomicCounterBuffers           = 0;
        resources.maxFragmentAtomicCounterBuffers           = 1;
        resources.maxCombinedAtomicCounterBuffers           = 1;
        resources.maxAtomicCounterBufferSize                = 16384;
        resources.maxTransformFeedbackBuffers               = 4;
        resources.maxTransformFeedbackInterleavedComponents = 64;
        resources.maxCullDistances                          = 8;
        resources.maxCombinedClipAndCullDistances           = 8;
        resources.maxSamples                                = 4;
        resources.maxMeshOutputVerticesNV                   = 256;
        resources.maxMeshOutputPrimitivesNV                 = 512;
        resources.maxMeshWorkGroupSizeX_NV                  = 32;
        resources.maxMeshWorkGroupSizeY_NV                  = 1;
        resources.maxMeshWorkGroupSizeZ_NV                  = 1;
        resources.maxTaskWorkGroupSizeX_NV                  = 32;
        resources.maxTaskWorkGroupSizeY_NV                  = 1;
        resources.maxTaskWorkGroupSizeZ_NV                  = 1;
        resources.maxMeshViewCountNV                        = 4;
        resources.maxMeshOutputVerticesEXT                  = 256;
        resources.maxMeshOutputPrimitivesEXT                = 256;
        resources.maxMeshWorkGroupSizeX_EXT                 = 128;
        resources.maxMeshWorkGroupSizeY_EXT                 = 128;
        resources.maxMeshWorkGroupSizeZ_EXT                 = 128;
        resources.maxTaskWorkGroupSizeX_EXT                 = 128;
        resources.maxTaskWorkGroupSizeY_EXT                 = 128;
        resources.maxTaskWorkGroupSizeZ_EXT                 = 128;
        resources.maxMeshViewCountEXT                       = 4;
        resources.maxDualSourceDrawBuffersEXT               = 1;

        resources.limits.nonInductiveForLoops                 = true;
        resources.limits.whileLoops                           = true;
        resources.limits.doWhileLoops                         = true;
        resources.limits.generalUniformIndexing               = true;
        resources.limits.generalAttributeMatrixVectorIndexing = true;
        resources.limits.generalVaryingIndexing               = true;
        resources.limits.generalSamplerIndexing               = true;
        resources.limits.generalVariableIndexing              = true;
        resources.limits.generalConstantMatrixVectorIndexing  = true;

        return resources;
    }

    static void fill_preprocess_error(const char* error_message, const Atomic::String& source_code, glslang::TShader* shader, ShaderCompilerPreProcessResult& output)
    {
        Atomic::String output_error(error_message);
        output_error.AppendWithFormat(": %s\n", shader->getInfoLog());
        output_error.AppendWithFormat("Debug Log: %s\n", shader->getInfoDebugLog());
        output_error.AppendWithFormat("Source: \n%s", source_code.CString());

        output.error = output_error;
        output.has_error = true;
        ::glslang::FinalizeProcess();
    }

    void shader_compiler_preprocess(const ShaderCompilerPreProcessDesc& desc, ShaderCompilerPreProcessResult& output)
    {
        ::glslang::InitializeProcess();
        const auto resources = init_resources();
        const char* shader_strings[] = { desc.source_code.CString() };
        const int shader_strings_len[] = {static_cast<int>(desc.source_code.Length())};

        constexpr auto messages = EShMsgSpvRules;

        const auto stage_type = get_stage_type(desc.type);
        ::glslang::TShader shader(stage_type);
        shader.setEnvInput(glslang::EShSourceGlsl, stage_type, glslang::EShClientVulkan, 100);
        shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
        shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_0);
        shader.setEntryPoint("main");
        shader.setStringsWithLengths(shader_strings, shader_strings_len, 1);

        shader.setAutoMapBindings(true);
        shader.setAutoMapLocations(true);

        std::string output_shader;
        glslang::TShader::ForbidIncluder includer = {};

    	auto result = shader.parse(&resources,
            100,
            ENoProfile,
            false,
            false,
            EShMsgDefault);

        Atomic::String error_output;
        if(!result)
        {
            fill_preprocess_error("Failed to parse shader source", desc.source_code, &shader, output);
            return;
        }


        result = shader.preprocess(&resources, 
            100, 
            ENoProfile, 
            false, 
            false, 
            messages, &output_shader, includer);

        if(!result)
        {
            fill_preprocess_error("Failed to parse preprocess shader source", desc.source_code, &shader, output);
            return;
        }

        ::glslang::FinalizeProcess();

        Atomic::String final_result;
        const auto code_parts = Atomic::String(output_shader.c_str(), output_shader.length()).Split('\n');
    	for(auto& line : code_parts)
        {
            if (line.Trimmed().Length() == 0)
                continue;
            final_result.Append(line);
            final_result.Append('\n');
        }

        output.has_error = false;
        output.source_code = final_result;
    }

}
