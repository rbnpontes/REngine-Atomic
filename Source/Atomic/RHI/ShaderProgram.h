#pragma once

#include "../Container/RefCounted.h"
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
        bool GetParameter(const Atomic::StringHash& paramName, Atomic::ShaderParameter* parameter);
        unsigned ToHash() { return hash_; }
    private:
        void CollectShaderParameters(const Atomic::ShaderVariation* shader);
        Atomic::Graphics* graphics_;
        Atomic::HashMap<Atomic::StringHash, Atomic::ShaderParameter> parameters_{};
        unsigned hash_{0};
    };
}
