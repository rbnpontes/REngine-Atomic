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

    struct ShaderSamplerDesc
    {
	    Atomic::String name;
        Atomic::StringHash hash;
    };

    class RENGINE_API ShaderProgram : public Atomic::RefCounted
    {
        ATOMIC_REFCOUNTED(ShaderProgram);
    public:
        ShaderProgram(const ShaderProgramCreationDesc& creation_desc);
        ~ShaderProgram() override;
        ShaderSamplerDesc* GetSampler(Atomic::TextureUnit unit) const;
        ShaderSamplerDesc* GetSampler(const Atomic::StringHash& name) const;
        unsigned ToHash() const { return hash_; }
        bool IsInUseTexture(const Atomic::StringHash& texture) const;
        const ea::vector<REngine::ShaderCompilerReflectInputElement>& GetInputElements() const { return input_elements_; }
    private:
        void CollectUsedInputElements(const Atomic::ShaderVariation* vertex_shader, const Atomic::ShaderVariation* pixel_shader);
        void CollectShaderParameters(const Atomic::ShaderVariation* shader) const;
        void CollectShaderTextures(const Atomic::ShaderVariation* shader);

        Atomic::Graphics* graphics_;
        ea::hash_map<u32, ea::shared_ptr<ShaderSamplerDesc>> used_textures_{};
        ea::vector<REngine::ShaderCompilerReflectInputElement> input_elements_{};
        ShaderSamplerDesc* used_texture_slot_names_[Atomic::MAX_TEXTURE_UNITS];
        unsigned hash_{0};
#if ENGINE_DEBUG
        // the properties is used only for debug purposes
        Atomic::String vs_shader_name_;
        Atomic::String ps_shader_name_;
#endif

    };
}
