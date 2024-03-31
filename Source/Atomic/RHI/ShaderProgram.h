#pragma once
#include "../Container/RefCounted.h"
#include "../Container/HashMap.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/ConstantBuffer.h"

namespace Atomic
{
	class ShaderVariation;
}
namespace REngine
{
    struct ShaderProgramCreationDesc
    {
        Atomic::Graphics* graphics{nullptr};
        Atomic::ShaderVariation* vertex_shader{nullptr};
        Atomic::ShaderVariation* pixel_shader{nullptr};

        uint32_t ToHash() const
        {
	        uint32_t hash = 0;
			if(vertex_shader)
				Atomic::CombineHash(hash, vertex_shader->ToHash());
			if(pixel_shader)
				Atomic::CombineHash(hash, pixel_shader->ToHash());
			return hash;
		}
    };
    class RENGINE_API ShaderProgram : public Atomic::RefCounted
    {
        ATOMIC_REFCOUNTED(ShaderProgram);
    public:
        ShaderProgram(const ShaderProgramCreationDesc& creation_desc);
        bool GetParameter(const Atomic::StringHash& paramName, Atomic::ShaderParameter* parameter);
        unsigned ToHash() const { return hash_; }
        bool IsInUseTexture(const Atomic::StringHash& texture) const { return used_textures_.Contains(texture); }
    private:
        void CollectShaderParameters(const Atomic::ShaderVariation* shader);
        void CollectShaderTextures(const Atomic::ShaderVariation* shader);
        Atomic::Graphics* graphics_;
        Atomic::HashMap<Atomic::StringHash, Atomic::ShaderParameter> parameters_{};
        Atomic::HashMap<Atomic::StringHash, Atomic::String> used_textures_{};
        unsigned hash_{0};
    };
}
