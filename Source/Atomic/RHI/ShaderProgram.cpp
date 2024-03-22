#include "./ShaderProgram.h"


namespace REngine
{
    static constexpr unsigned s_max_constant_buffers = static_cast<unsigned>(Atomic::MAX_SHADER_PARAMETER_GROUPS *
        Atomic::MAX_SHADER_TYPES);

    ShaderProgram::ShaderProgram(const ShaderProgramCreationDesc& creation_desc) :
        RefCounted(),
        graphics_(creation_desc.graphics)
    {
        assert(creation_desc.vertex_shader && creation_desc.vertex_shader,
               "Vertex Shader and Pixel Shader is Required");

        CollectConstantBuffer(creation_desc.vertex_shader);
        CollectConstantBuffer(creation_desc.pixel_shader);

        CollectShaderParameters(creation_desc.vertex_shader);
        CollectShaderParameters(creation_desc.pixel_shader);

        parameters_.Rehash(Atomic::NextPowerOfTwo(parameters_.Size()));

        hash_ = Atomic::MakeHash(creation_desc.vertex_shader);
        Atomic::CombineHash(hash_, Atomic::MakeHash(creation_desc.pixel_shader));
    }

    void ShaderProgram::CollectConstantBuffer(const Atomic::ShaderVariation* shader)
    {
        const auto buffer_size = shader->GetConstantBufferSizes();
        const auto type = shader->GetShaderType();
        for (unsigned i = 0; i < Atomic::MAX_SHADER_PARAMETER_GROUPS; ++i)
        {
            if (buffer_size[i] == 0)
                continue;
            const auto c_buffer = graphics_->GetOrCreateConstantBuffer(type, i, buffer_size[i]);
            SetConstantBuffer(type, static_cast<Atomic::ShaderParameterGroup>(i), c_buffer);
        }
    }

    void ShaderProgram::CollectShaderParameters(const Atomic::ShaderVariation* shader)
    {
        const auto& params = shader->GetParameters();
        for (const auto& it : params)
        {
            parameters_[it.first_] = it.second_;
            parameters_[it.first_].bufferPtr_
                = GetConstantBuffer(shader->GetShaderType(),
                                    static_cast<Atomic::ShaderParameterGroup>(it.second_.buffer_));
        }
    }

    Atomic::SharedPtr<Atomic::ConstantBuffer> ShaderProgram::GetConstantBuffer(
        Atomic::ShaderType type, Atomic::ShaderParameterGroup grp)
    {
        return constant_buffers_[static_cast<unsigned>(grp) * static_cast<unsigned>(type)];
    }

    void ShaderProgram::SetConstantBuffer(Atomic::ShaderType type, Atomic::ShaderParameterGroup grp,
                                          Atomic::ConstantBuffer* constant_buffer)
    {
        constant_buffers_[static_cast<unsigned>(grp) * static_cast<unsigned>(type)] = constant_buffer;
    }
}
