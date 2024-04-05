#include "./GraphicsState.h"

namespace REngine
{
    static GraphicsState s_state = {};
    static RenderCommandState s_default_command = {};

    const GraphicsState& graphics_state_get()
    {
        return s_state;
    }

    void graphics_state_set(const GraphicsState state)
    {
        s_state = state;
    }

    Atomic::SharedPtr<Atomic::ConstantBuffer> graphics_state_get_constant_buffer(Atomic::ShaderType type, uint32_t size)
    {
        uint32_t hash = size;
        Atomic::CombineHash(hash, static_cast<uint32_t>(type));
        const auto& it = s_state.constant_buffers.Find(hash);
        if(it == s_state.constant_buffers.End())
			return {};
        return it->second_;
    }

    void graphics_state_set_constant_buffer(const ConstantBufferCacheDesc& desc)
    {
		s_state.constant_buffers[desc.ToHash()] = desc.constant_buffer;
    }

    void graphics_state_release_constant_buffers()
    {
		s_state.constant_buffers.Clear();
    }

    uint32_t graphics_state_constant_buffers_count()
    {
		return s_state.constant_buffers.Size();
    }

    Atomic::SharedPtr<ShaderProgram> graphics_state_get_shader_program(const ShaderProgramQuery& query)
    {
        const auto hash = query.ToHash();

        const auto& it = s_state.shader_programs.Find(hash);
        if(it == s_state.shader_programs.End())
            return {};
        return it->second_;
    }
    void graphics_state_set_shader_program(const Atomic::SharedPtr<ShaderProgram> program)
    {
        s_state.shader_programs[program->ToHash()] = program;
    }

    uint32_t graphics_state_shader_programs_count()
    {
        return s_state.shader_programs.Size();
    }

    void graphics_state_release_shader_programs()
    {
    	s_state.shader_programs.Clear();
    }

    Atomic::SharedPtr<VertexDeclaration> graphics_state_get_vertex_declaration(const uint32_t id)
    {
	    const auto it = s_state.vertex_declarations.Find(id);
        if(it == s_state.vertex_declarations.End())
			return {};
        return it->second_;
    }

    void graphics_state_set_vertex_declaration(const uint32_t id, const Atomic::SharedPtr<VertexDeclaration>& declaration)
    {
        s_state.vertex_declarations[id] = declaration;
    }

    void graphics_state_release_vertex_declarations()
    {
    	s_state.vertex_declarations.Clear();
    }

    uint32_t graphics_state_vertex_declarations_count()
    {
        return s_state.vertex_declarations.Size();
    }


    RenderCommandState* default_render_command_get()
    {
        return &s_default_command;
    }
}
