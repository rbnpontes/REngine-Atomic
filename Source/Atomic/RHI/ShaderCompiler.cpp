#include "./ShaderCompiler.h"
#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

#include "IO/Log.h"


namespace REngine
{
    static glslang_stage_t get_stage_type(Atomic::ShaderType type)
    {
        switch (type) {
        case Atomic::VS:
            return GLSLANG_STAGE_VERTEX;
        case Atomic::PS:
            return GLSLANG_STAGE_FRAGMENT;
        }
        ATOMIC_LOGERROR("Invalid Shader Type");
        return GLSLANG_STAGE_COUNT;
    }
    void shader_compiler_preprocess(const ShaderCompilerPreProcessDesc& desc, ShaderCompilerPreProcessResult& output)
    {
        glslang_input_t input = {};
        input.language = GLSLANG_SOURCE_GLSL;
        input.stage = get_stage_type(desc.type);
        input.client = GLSLANG_CLIENT_OPENGL;
        input.client_version = GLSLANG_TARGET_OPENGL_450;
        input.target_language = GLSLANG_TARGET_SPV;
        input.target_language_version = GLSLANG_TARGET_SPV_1_2;
        input.code = desc.source_code.CString();
        input.default_version = 450;
        input.default_profile = GLSLANG_NO_PROFILE;
        input.force_default_version_and_profile = false;
        input.forward_compatible = false;
        input.messages = GLSLANG_MSG_DEFAULT_BIT;
        input.resource = glslang_default_resource();
        
        glslang_shader_t* shader = glslang_shader_create(&input);

        if(!glslang_shader_preprocess(shader, &input))
        {
            Atomic::String error;
            error.Append(glslang_shader_get_info_log(shader));
            error.Append("\n");
            error.Append(glslang_shader_get_info_debug_log(shader));
            error.Append("\n");
            error.Append(input.code);

            glslang_shader_delete(shader);
            output.has_error = true;
            return;
        }

        output.has_error = false;
        output.source_code = input.code;
    }

}
