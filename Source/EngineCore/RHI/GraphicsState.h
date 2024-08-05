#pragma once
#include "./RenderCommand.h"
#include "../Container/HashMap.h"
#include "../Graphics/ConstantBuffer.h"
#include "../Container/Ptr.h"
#include "./ShaderProgram.h"
#include "./VertexDeclaration.h"

namespace REngine
{
    using namespace Diligent;
    struct GraphicsState
    {
        Atomic::HashMap<uint32_t, Atomic::SharedPtr<REngine::ShaderProgram>> shader_programs;
        Atomic::HashMap<uint32_t, Atomic::SharedPtr<REngine::VertexDeclaration>> vertex_declarations;
        Atomic::HashMap<uint32_t, Atomic::SharedPtr<Atomic::ConstantBuffer>> constant_buffers;
    };

    struct ShaderProgramQuery
    {
        Atomic::ShaderVariation* vertex_shader{nullptr};
        Atomic::ShaderVariation* pixel_shader{nullptr};
        Atomic::ShaderVariation* geometry_shader{nullptr};
        Atomic::ShaderVariation* hull_shader{nullptr};
        Atomic::ShaderVariation* domain_shader{nullptr};

        u32 ToHash() const
        {
	        u32 hash = 0;
            if(vertex_shader)
	            Atomic::CombineHash(hash, vertex_shader->ToHash());
            if(pixel_shader)
			    Atomic::CombineHash(hash, pixel_shader->ToHash());
            if(geometry_shader)
                Atomic::CombineHash(hash, geometry_shader->ToHash());
            if(hull_shader)
                Atomic::CombineHash(hash, hull_shader->ToHash());
            if (domain_shader)
                Atomic::CombineHash(hash, domain_shader->ToHash());
			return hash;
		}
    };

    struct ConstantBufferCacheDesc
    {
        Atomic::SharedPtr<Atomic::ConstantBuffer> constant_buffer{};
        Atomic::ShaderType type{Atomic::MAX_SHADER_TYPES};

        uint32_t ToHash() const
        {
            uint32_t hash = constant_buffer->GetSize();
            Atomic::CombineHash(hash, static_cast<uint32_t>(type));
            return hash;
        }
    };
    
    const GraphicsState& graphics_state_get();
    void graphics_state_set(const GraphicsState state);

    Atomic::SharedPtr<Atomic::ConstantBuffer> graphics_state_get_constant_buffer(Atomic::ShaderType type, uint32_t size);
    void graphics_state_set_constant_buffer(const ConstantBufferCacheDesc& desc);
    void graphics_state_release_constant_buffers();
    uint32_t graphics_state_constant_buffers_count();

    Atomic::SharedPtr<ShaderProgram> graphics_state_get_shader_program(const ShaderProgramQuery& query);
    RENGINE_API void graphics_state_set_shader_program(const Atomic::SharedPtr<ShaderProgram> program);
    uint32_t graphics_state_shader_programs_count();
    void graphics_state_release_shader_programs();

    Atomic::SharedPtr<VertexDeclaration> graphics_state_get_vertex_declaration(const uint32_t id);
    void graphics_state_set_vertex_declaration(const uint32_t id, const Atomic::SharedPtr<VertexDeclaration>& declaration);
    void graphics_state_release_vertex_declarations();
    uint32_t graphics_state_vertex_declarations_count();
}
