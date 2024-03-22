#pragma once

#include "../Container/RefCounted.h"
#include "../Container/Ptr.h"
#include "../Container/HashMap.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/ConstantBuffer.h"
#include "../Graphics/ShaderVariation.h"

namespace REngine
{
    struct ShaderProgramCreationDesc
    {
        Atomic::Graphics* graphics{nullptr};
        Atomic::ShaderVariation* vertex_shader{nullptr};
        Atomic::ShaderVariation* pixel_shader{nullptr};
    };
    class RENGINE_API ShaderProgram : public Atomic::RefCounted
    {
        ATOMIC_REFCOUNTED(ShaderProgram);
    public:
        ShaderProgram(const ShaderProgramCreationDesc& creation_desc);
        Atomic::SharedPtr<Atomic::ConstantBuffer> GetConstantBuffer(Atomic::ShaderType type, Atomic::ShaderParameterGroup grp);
        unsigned ToHash() { return hash_; }
    private:
        void CollectConstantBuffer(const Atomic::ShaderVariation* shader);
        void CollectShaderParameters(const Atomic::ShaderVariation* shader);
        void SetConstantBuffer(Atomic::ShaderType type, Atomic::ShaderParameterGroup grp, Atomic::ConstantBuffer* constant_buffer);
        Atomic::Graphics* graphics_;
        Atomic::HashMap<Atomic::StringHash, Atomic::ShaderParameter> parameters_{};
        Atomic::SharedPtr<Atomic::ConstantBuffer> constant_buffers_[Atomic::MAX_SHADER_PARAMETER_GROUPS * Atomic::MAX_SHADER_TYPES]{};
        unsigned hash_{0};
    };
}
