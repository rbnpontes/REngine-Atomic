#include "./ShaderProgram.h"
#include "../Graphics/ShaderVariation.h"
#include "./DriverInstance.h"

namespace REngine
{
    static constexpr unsigned s_max_constant_buffers = static_cast<unsigned>(Atomic::MAX_SHADER_PARAMETER_GROUPS *
        Atomic::MAX_SHADER_TYPES);

    ShaderProgram::ShaderProgram(const ShaderProgramCreationDesc& creation_desc) :
	    graphics_(creation_desc.graphics)
    {
        assert(creation_desc.vertex_shader && creation_desc.vertex_shader,
               "Vertex Shader and Pixel Shader is Required");

        CollectShaderParameters(creation_desc.vertex_shader);
        CollectShaderParameters(creation_desc.pixel_shader);
        CollectShaderTextures(creation_desc.vertex_shader);
        CollectShaderTextures(creation_desc.pixel_shader);

        parameters_.Rehash(Atomic::NextPowerOfTwo(parameters_.Size()));

        hash_ = creation_desc.ToHash();
    }

    void ShaderProgram::CollectShaderParameters(const Atomic::ShaderVariation* shader)
    {
        const auto& params = shader->GetParameters();
        for (const auto& it : params)
        {
            Atomic::ShaderParameter shader_param = it.second_;
            shader_param.bufferPtr_ = graphics_
				->GetImpl()
				->GetConstantBuffer(shader->GetShaderType(), static_cast<Atomic::ShaderParameterGroup>(it.second_.buffer_));
            parameters_[it.first_] = shader_param;
        }
    }

    void ShaderProgram::CollectShaderTextures(const Atomic::ShaderVariation* shader)
    {
        const auto& textures = shader->GetUseTextureNames();
		for (const auto& it : textures)
			used_textures_[it] = it;
	}

    bool ShaderProgram::GetParameter(const Atomic::StringHash& paramName, Atomic::ShaderParameter* parameter)
    {
	    const auto it = parameters_.Find(paramName);
        if(it == parameters_.End())
			return false;
		*parameter = it->second_;
		return true;
    }
}
