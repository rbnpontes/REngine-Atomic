// require this define:
// https://github.com/microsoft/cppwinrt/issues/479
// otherwise spirv_cross will crash
#define NOMINMAX

#include "./ShaderCompiler.h"
#include "./DiligentUtils.h"

#include "../Engine/Application.h"
#include "../IO/Log.h"

#include "spirv_parser.hpp"
#include "spirv_cross.hpp"

#include <glslang/Public/ShaderLang.h>

#include <SPIRV/GlslangToSpv.h>
#include <DiligentCore/Graphics/ShaderTools/include/SPIRVTools.hpp>

namespace REngine
{
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

    static void fill_error(const char* error_message, const Atomic::String& source_code, glslang::TShader* shader, Atomic::String& output)
    {
        Atomic::String output_error(error_message);
        output_error.AppendWithFormat(": %s\n", shader->getInfoLog());
        output_error.AppendWithFormat("Debug Log: %s\n", shader->getInfoDebugLog());
        output_error.AppendWithFormat("Source: \n%s", source_code.CString());

        output = output_error;
        ::glslang::FinalizeProcess();
    }
    static void fill_error(const char* error_message, const Atomic::String& source_code, glslang::TProgram* program, Atomic::String& output)
    {
        Atomic::String output_error(error_message);
        output_error.AppendWithFormat(": %s\n", program->getInfoLog());
        output_error.AppendWithFormat("Debug Log: %s\n", program->getInfoDebugLog());
        output_error.AppendWithFormat("Source: \n%s", source_code.CString());

        output = output_error;
        ::glslang::FinalizeProcess();
    }

    static Atomic::VertexElementType get_element_type(const spirv_cross::SPIRType& type)
    {
        Atomic::VertexElementType result = Atomic::MAX_VERTEX_ELEMENT_TYPES;
	    switch (type.basetype)
    	{
        case spirv_cross::SPIRType::Int:
            result = TYPE_INT;
            break;
        case spirv_cross::SPIRType::Float:
            result = TYPE_FLOAT;
            break;
        default:
            if (type.columns > 1) {
                switch (type.vecsize) {
                    case 2:
                        result = TYPE_VECTOR2;
                        break;
                    case 3:
                        result = TYPE_VECTOR3;
                        break;
                    case 4:
                        result = TYPE_VECTOR4;
                        break;
                }
            } else if(type.basetype == spirv_cross::SPIRType::UInt && type.width == 8 && type.columns == 1) {
                result = type.array.empty() ? TYPE_UBYTE4 : TYPE_UBYTE4_NORM;
            }
        }

        return result;
    }

    void shader_compiler_preprocess(const ShaderCompilerDesc& desc, ShaderCompilerPreProcessResult& output)
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

        if(!result)
        {
            fill_error("Failed to parse shader source", desc.source_code, &shader, output.error);
            output.has_error = true;
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
            fill_error("Failed to parse preprocess shader source", desc.source_code, &shader, output.error);
            output.has_error = true;
            return;
        }

        ::glslang::FinalizeProcess();

        Atomic::String final_result;
        const Atomic::StringVector code_parts = Atomic::String(output_shader.c_str(), output_shader.length()).Split('\n');
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

    void shader_compiler_compile(const ShaderCompilerDesc& desc, const bool optimize, ShaderCompilerResult& output)
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

        auto result = shader.parse(&resources,
            100,
            ENoProfile,
            false,
            false,
            EShMsgDefault);

        if(!result)
        {
            fill_error("Failed to parse shader source", desc.source_code, &shader, output.error);
            output.has_error = true;
            return;     
        }

        ::glslang::TProgram program;
        program.addShader(&shader);
        if(!program.link(messages))
        {
            fill_error("Failed to link program", desc.source_code, &program, output.error);
            output.has_error = true;
            return;
        }

        program.mapIO();

        std::vector<unsigned int> spirv_code;
        ::glslang::GlslangToSpv(*program.getIntermediate(stage_type), spirv_code);
        ::glslang::FinalizeProcess();

        if (spirv_code.empty())
        {
            output.has_error = false;
            return;
        }

        if (optimize)
            spirv_code = Diligent::OptimizeSPIRV(spirv_code, SPV_ENV_VULKAN_1_0, Diligent::SPIRV_OPTIMIZATION_FLAG_PERFORMANCE);

        output.spirv_code = Atomic::PODVector<uint8_t>(
            static_cast<uint8_t*>(static_cast<void*>(spirv_code.data())),
            spirv_code.size() * sizeof(unsigned int)
        );
        output.has_error = false;
    }

    void shader_compiler_reflect(const ShaderCompilerReflectDesc& desc, ShaderCompilerReflectInfo& output)
    {
        if (desc.length == 0 || desc.spirv_code == nullptr)
            return;

        spirv_cross::Parser parser{ static_cast<uint32_t*>(desc.spirv_code), desc.length / sizeof(uint32_t) };
        parser.parse();
        spirv_cross::Compiler compiler{ std::move(parser.get_parsed_ir()) };
        auto resources = compiler.get_shader_resources();

    	for(const auto& uniform_buffer : resources.uniform_buffers)
        {
            const auto& type = compiler.get_type(uniform_buffer.base_type_id);
            const auto& name = Atomic::String(uniform_buffer.name.c_str());
            const auto& grp_type = utils_get_shader_parameter_group_type(name);
            const auto& buffer_size = compiler.get_declared_struct_size(type);

            ShaderCompilerConstantBufferDesc buffer_desc = {};
            buffer_desc.name = name;
            buffer_desc.size = static_cast<uint32_t>(buffer_size);
            output.constant_buffers[name] = buffer_desc;

            if (grp_type != Atomic::MAX_SHADER_PARAMETER_GROUPS)
                output.constant_buffer_sizes[grp_type] = buffer_desc.size;

            for(uint32_t i = 0; i < type.member_types.size(); ++i)
            {
                const auto& member_type_id = type.member_types[i];
            	auto member_name = Atomic::String(compiler.get_member_name(uniform_buffer.base_type_id, member_type_id).c_str());
                const auto& member_offset = compiler.get_member_decoration(uniform_buffer.base_type_id, i, spv::DecorationOffset);
                const auto& member_size = compiler.get_declared_struct_member_size(type, i);

                // 'c' is a member naming convention on Urho3D/Atomic, we need to get rid this.
                if (member_name.Length() > 0 && member_name[0] == 'c')
                    member_name = member_name.Substring(1);

                Atomic::ShaderParameter shader_param = {};
                shader_param.buffer_ = buffer_desc.parameter_group;
                shader_param.name_ = member_name;
                shader_param.type_ = desc.type;
                shader_param.offset_ = member_offset;
                shader_param.size_ = static_cast<uint32_t>(member_size);

                output.parameters[member_name] = shader_param;
            }
        }

        output.input_elements.Resize(resources.stage_inputs.size());
        unsigned idx = 0;
        for(const auto& input : resources.stage_inputs)
        {
            const auto type = compiler.get_type(input.type_id);
            const auto name = Atomic::String(compiler.get_name(input.id).c_str());
            const auto& location = compiler.get_decoration(input.id, spv::DecorationLocation);
            uint8_t semantic_index = 0;
            const auto semantic = utils_get_element_semantic(name, &semantic_index);
            const auto vertex_type = get_element_type(type);

            ShaderCompilerReflectInputElement element = {};
            element.index = static_cast<uint8_t>(location);
            element.semantic_index = semantic_index;
            element.semantic = semantic;
            element.element_type = vertex_type;
            element.name = name;

            output.input_elements[idx] = element;
        }

        idx = 0;
        memset(&output.used_texture_units, 0x0, sizeof(bool) * MAX_TEXTURE_UNITS);
        for(const auto& image : resources.sampled_images)
        {
            const auto name = Atomic::String(compiler.get_name(image.id).c_str());
            const auto texture_unit = utils_get_texture_unit(name);

            if (texture_unit != MAX_TEXTURE_UNITS)
                output.used_texture_units[texture_unit] = true;
            output.samplers.Push(name);
        }
    }
}
