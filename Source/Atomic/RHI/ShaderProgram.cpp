#include "./ShaderProgram.h"
#include "../Graphics/ShaderVariation.h"
#include "./DriverInstance.h"
#include "./DiligentUtils.h"
#include <Windows.h>

namespace REngine
{
    static constexpr unsigned s_max_constant_buffers = static_cast<unsigned>(Atomic::MAX_SHADER_PARAMETER_GROUPS *
        Atomic::MAX_SHADER_TYPES);

    ShaderProgram::ShaderProgram(const ShaderProgramCreationDesc& creation_desc) :
	    graphics_(creation_desc.graphics)
    {
        assert(creation_desc.vertex_shader && creation_desc.vertex_shader,
               "Vertex Shader and Pixel Shader is Required");

        memset(used_texture_slot_names_, 0, sizeof(void*) * Atomic::MAX_TEXTURE_UNITS);
#if ATOMIC_DEBUG
        vs_shader_name_ = creation_desc.vertex_shader->GetName();
        ps_shader_name_ = creation_desc.pixel_shader->GetName();
#endif
        /*if (creation_desc.vertex_shader->GetName().Contains("Water"))
            ::DebugBreak();*/
        CollectShaderParameters(creation_desc.vertex_shader);
        CollectShaderParameters(creation_desc.pixel_shader);
        CollectShaderTextures(creation_desc.vertex_shader);
        CollectShaderTextures(creation_desc.pixel_shader);

        parameters_.Rehash(Atomic::NextPowerOfTwo(parameters_.Size()));

        hash_ = creation_desc.ToHash();
    }

    ShaderProgram::~ShaderProgram()
    {
        for(const auto& it : used_textures_)
			delete it.second_;
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
		{
			const auto tex_unit = utils_get_texture_unit(it);
			const auto sampler_desc = new ShaderSamplerDesc();
            sampler_desc->name = it;
            sampler_desc->hash = it;
			used_textures_[sampler_desc->hash] = sampler_desc;
            if(tex_unit != Atomic::MAX_TEXTURE_UNITS)
				used_texture_slot_names_[tex_unit] = sampler_desc;
		}
	}

    bool ShaderProgram::GetParameter(const Atomic::StringHash& paramName, Atomic::ShaderParameter* parameter)
    {
	    const auto it = parameters_.Find(paramName);
        if(it == parameters_.End())
			return false;
		*parameter = it->second_;
		return true;
    }


    ShaderSamplerDesc* ShaderProgram::GetSampler(Atomic::TextureUnit unit) const
    {
        if(unit >= Atomic::MAX_TEXTURE_UNITS)
            return nullptr;
        return used_texture_slot_names_[unit];
    }

    ShaderSamplerDesc* ShaderProgram::GetSampler(const Atomic::StringHash& name) const
    {
        const auto it = used_textures_.Find(name);
        if(it == used_textures_.End())
            return nullptr;
        return it->second_;
    }
}
