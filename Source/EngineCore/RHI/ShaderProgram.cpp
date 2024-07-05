#include "./ShaderProgram.h"
#include "../Graphics/ShaderVariation.h"
#include "./DriverInstance.h"
#include "./DiligentUtils.h"
#include "./ShaderParametersCache.h"

#if ENGINE_PLATFORM_IOS
    #include <OpenGLES/gltypes.h>
    #include <OpenGLES/ES3/gl.h>
#elif ENGINE_PLATFORM_ANDROID
    #include <GLES3/gl3.h>
#else
    #include <GLEW/glew.h>
#endif

#include <DiligentCore/Graphics/GraphicsEngineOpenGL/interface/ShaderGL.h>

namespace REngine
{
    static constexpr unsigned s_max_constant_buffers = static_cast<unsigned>(Atomic::MAX_SHADER_PARAMETER_GROUPS *
        Atomic::MAX_SHADER_TYPES);

    ShaderProgram::ShaderProgram(const ShaderProgramCreationDesc& creation_desc) :
	    graphics_(creation_desc.graphics)
    {
        memset(used_texture_slot_names_, 0, sizeof(void*) * Atomic::MAX_TEXTURE_UNITS);
#if ENGINE_DEBUG
        vs_shader_name_ = creation_desc.vertex_shader->GetName();
        ps_shader_name_ = creation_desc.pixel_shader->GetName();
#endif

        CollectUsedInputElements(creation_desc.vertex_shader, creation_desc.pixel_shader);
        CollectShaderParameters(creation_desc.vertex_shader);
        CollectShaderParameters(creation_desc.pixel_shader);
        CollectShaderTextures(creation_desc.vertex_shader);
        CollectShaderTextures(creation_desc.pixel_shader);

        hash_ = creation_desc.ToHash();
    }

    ShaderProgram::~ShaderProgram()
    = default;

    bool ShaderProgram::IsInUseTexture(const Atomic::StringHash& texture) const
    {
    	return used_textures_.find_as(texture.Value()) != used_textures_.end();
    }


    void ShaderProgram::CollectUsedInputElements(const Atomic::ShaderVariation* vertex_shader, const Atomic::ShaderVariation* pixel_shader) {
        // OpenGL strips unused attributes when vertex shader and fragment shader
        // is linked into GL Shader Program. 
        const auto backend = graphics_->GetBackend();
        if(backend != GraphicsBackend::OpenGL && backend != GraphicsBackend::OpenGLES)
        {
            input_elements_ = vertex_shader->GetInputElements();
            return;
        }

        const auto vs = vertex_shader->GetGPUObject().Cast<Diligent::IShaderGL>(Diligent::IID_ShaderGL);
        const auto ps = pixel_shader->GetGPUObject().Cast<Diligent::IShaderGL>(Diligent::IID_ShaderGL);

        const auto tmp_program = glCreateProgram();
        glAttachShader(tmp_program, vs->GetGLShaderHandle());
        glAttachShader(tmp_program, ps->GetGLShaderHandle());
        glLinkProgram(tmp_program);

        // Reflection already lists all vertex attributes for us
        // But order and used inputs is not guaranteed, in this case
        // we will remap all location based on compiled GL shader.
        // excluding the unused ones.

        ea::hash_map<u32, REngine::ShaderCompilerReflectInputElement> elements;
        // fill input elements on hash map for fast lookup
    	for (auto& it : vertex_shader->GetInputElements())
            elements[StringHash(it.name).Value()] = it;

        GLint num_attrs;
        GLint max_attr_length;
        glGetProgramiv(tmp_program, GL_ACTIVE_ATTRIBUTES, &num_attrs);
        glGetProgramiv(tmp_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_attr_length);

        ea::vector<char> buffer(max_attr_length);
        // Loop for each active attribute
        // And update current input layout list.
        for(GLint i = 0; i < num_attrs; ++i)
        {
            GLsizei length;
            GLint size;
            GLenum type;
            glGetActiveAttrib(tmp_program, i, static_cast<GLsizei>(buffer.size()), &length, &size, &type, buffer.data());
            GLint location = glGetAttribLocation(tmp_program, buffer.data());
            StringHash name_hash = String(buffer.data(), length);

            const auto it = elements.find_as(name_hash.Value());
            if (it == elements.end())
                continue;

            auto& element = it->second;
            element.index = static_cast<u8>(location);
            input_elements_.push_back(element);
        }

        glDeleteProgram(tmp_program);
    };

    void ShaderProgram::CollectShaderParameters(const Atomic::ShaderVariation* shader) const
    {
        const auto& params = shader->GetParameters();
        for (const auto& it : params)
        {
            auto shader_param = it.second_;
            shader_param.bufferPtr_ = graphics_
				->GetImpl()
				->GetConstantBuffer(shader->GetShaderType(), static_cast<Atomic::ShaderParameterGroup>(it.second_.buffer_));
            REngine::shader_parameters_cache_add(it.first_.Value(), shader_param);
        }
    }

    void ShaderProgram::CollectShaderTextures(const Atomic::ShaderVariation* shader)
    {
        const auto& textures = shader->GetUseTextureNames();
		for (const auto& it : textures)
		{
			const auto tex_unit = utils_get_texture_unit(it);
			const auto sampler_desc = ea::make_shared<ShaderSamplerDesc>();
            sampler_desc->name = it;
            sampler_desc->hash = it;
			used_textures_[sampler_desc->hash.Value()] = sampler_desc;
            if(tex_unit != Atomic::MAX_TEXTURE_UNITS)
				used_texture_slot_names_[tex_unit] = sampler_desc.get();
		}
	}


    ShaderSamplerDesc* ShaderProgram::GetSampler(Atomic::TextureUnit unit) const
    {
        if(unit >= Atomic::MAX_TEXTURE_UNITS)
            return nullptr;
        return used_texture_slot_names_[unit];
    }

    ShaderSamplerDesc* ShaderProgram::GetSampler(const Atomic::StringHash& name) const
    {
        const auto it = used_textures_.find_as(name.Value());
        if(it == used_textures_.end())
            return nullptr;
        return it->second.get();
    }
}
