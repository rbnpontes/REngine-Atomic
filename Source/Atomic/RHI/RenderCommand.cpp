#include "./RenderCommand.h"

#include "./DiligentUtils.h"
#include "../IO/Log.h"
#include "../Core/Profiler.h"

namespace REngine
{
    static Diligent::ITextureView* s_tmp_render_targets[Atomic::MAX_RENDERTARGETS];
    static Diligent::IBuffer* s_tmp_vertex_buffers[Atomic::MAX_VERTEX_STREAMS];

    void render_command_process(const RenderCommandProcessDesc& desc, RenderCommandState* state)
    {
        const auto graphics = desc.graphics;
        const auto driver = desc.driver;
        const auto context = driver->GetDeviceContext();

        // resolve render target and depth stencil formats
        const auto num_rts = Atomic::Min(state->num_rts, Atomic::MAX_RENDERTARGETS);
        state->pipeline_state_info.output.num_rts = num_rts;

        {
            ATOMIC_PROFILE(RenderCommand::SetupRTs);
            for (unsigned i = 0; i < num_rts; ++i)
            {
                assert(state->render_targets[i]);
                const auto rt_format = state->render_targets[i]->GetDesc().Format;
                if (rt_format != state->pipeline_state_info.output.render_target_formats[i])
                    state->dirty_state |= static_cast<unsigned>(RenderCommandDirtyState::pipeline);
                state->pipeline_state_info.output.render_target_formats[i] = rt_format;
                s_tmp_render_targets[i] = state->render_targets[i];
            }

            auto depth_fmt = Diligent::TEX_FORMAT_UNKNOWN;
            if (state->depth_stencil)
                depth_fmt = state->depth_stencil->GetDesc().Format;

            if (depth_fmt != state->pipeline_state_info.output.depth_stencil_format)
                state->dirty_state |= static_cast<unsigned>(RenderCommandDirtyState::pipeline);
            state->pipeline_state_info.output.depth_stencil_format = depth_fmt;
        }

        {
	        
            ATOMIC_PROFILE(RenderCommand::PipelineBuild);
            if(state->skip_flags & static_cast<unsigned>(RenderCommandSkipFlags::pipeline_build))
                state->skip_flags ^= static_cast<unsigned>(RenderCommandSkipFlags::pipeline_build);
            // build pipeline if is necessary
            else if ((state->dirty_state & static_cast<unsigned>(RenderCommandDirtyState::pipeline)) != 0)
            {
                auto pipeline_hash = state->pipeline_state_info.ToHash();
                if (pipeline_hash != state->pipeline_hash || state->pipeline_state == nullptr)
                {
                    state->pipeline_hash = pipeline_hash;
                    state->pipeline_state = pipeline_state_builder_acquire(driver, state->pipeline_state_info, pipeline_hash);
                    state->shader_resource_binding = nullptr;
                }

                if(pipeline_hash != 0 && state->pipeline_state)
                    state->dirty_state ^= static_cast<unsigned>(RenderCommandDirtyState::pipeline);
            }
        }

        {
            ATOMIC_PROFILE(RenderCommand::BuildSRB);
            if(state->skip_flags & static_cast<unsigned>(RenderCommandSkipFlags::srb_build))
                state->skip_flags ^= static_cast<unsigned>(RenderCommandSkipFlags::srb_build);
            // build shader resource binding if is necessary
            else if(state->pipeline_state)
            {
                ShaderResourceBindingCreateDesc srb_desc;
                srb_desc.resources = &state->textures;
                srb_desc.driver = driver;
                srb_desc.pipeline_hash = state->pipeline_hash;
                state->shader_resource_binding = pipeline_state_builder_get_or_create_srb(srb_desc);
            }
        }

        {
            ATOMIC_PROFILE(RenderCommand::SetRTs);
            // bind render targets if is necessary
            context->SetRenderTargets(num_rts, s_tmp_render_targets, state->depth_stencil,
                                        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }
        {
            ATOMIC_PROFILE(RenderCommand::SetStencilRef);
            context->SetStencilRef(state->stencil_ref);
        }

        {
            ATOMIC_PROFILE(RenderCommand::SetBlendFactors);
            static constexpr float s_blend_factors[] = {.0f, .0f, .0f, .0f};
            context->SetBlendFactors(s_blend_factors);
        }

        {
            ATOMIC_PROFILE(RenderCommand::SetViewport);
            if(state->dirty_state & static_cast<unsigned>(RenderCommandDirtyState::viewport))
            {
                const auto rect = state->viewport;
                Diligent::Viewport viewport;
                viewport.TopLeftX = static_cast<float>(rect.left_);
                viewport.TopLeftY = static_cast<float>(rect.top_);
                viewport.Width = static_cast<float>(rect.right_ - rect.left_);
                viewport.Height = static_cast<float>(rect.bottom_ - rect.top_);
                viewport.MinDepth = 0.0f;
                viewport.MaxDepth = 1.0f;
                context->SetViewports(1, &viewport, graphics->GetWidth(), graphics->GetHeight());

                state->dirty_state ^= static_cast<unsigned>(RenderCommandDirtyState::viewport);
            }
        }

        {
	        
            ATOMIC_PROFILE(RenderCommand::SetScissors);
            if(state->dirty_state & static_cast<unsigned>(RenderCommandDirtyState::scissor) && 
                state->pipeline_state_info.scissor_test_enabled)
            {
                const auto rect = state->scissor;
                const Diligent::Rect scissor = { rect.left_, rect.top_, rect.right_, rect.bottom_};
                context->SetScissorRects(1, &scissor, graphics->GetWidth(), graphics->GetHeight());

                state->dirty_state ^= static_cast<unsigned>(RenderCommandDirtyState::scissor);
            }
        }

        {
            ATOMIC_PROFILE(RenderCommand::SetVertexBuffers);
            if(state->dirty_state & static_cast<unsigned>(RenderCommandDirtyState::vertex_buffer))
            {
                unsigned next_idx =0;
                for(const auto& buffer : state->vertex_buffers)
                {
                    if(!buffer)
                        continue;
                    s_tmp_vertex_buffers[next_idx] = buffer;
                    ++next_idx;
                }

                if(next_idx > 0)
                    context->SetVertexBuffers(
                        0,
                        next_idx,
                        s_tmp_vertex_buffers,
                        state->vertex_offsets,
                        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                state->dirty_state ^= static_cast<unsigned>(RenderCommandDirtyState::vertex_buffer);
            }
        }

        {
	        ATOMIC_PROFILE(RenderCommand::SetIndexBuffer);
            context->SetIndexBuffer(state->index_buffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }

        {

        	ATOMIC_PROFILE(RenderCommand::SetPipelineState);
            if(state->pipeline_state)
            {
                context->SetPipelineState(state->pipeline_state);
            }
        }

        {
            ATOMIC_PROFILE(RenderCommand::CommitSRB);
            if (state->shader_resource_binding)
            {
                context->CommitShaderResources(state->shader_resource_binding, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            }
            else if(state->pipeline_state)
                ATOMIC_LOGERROR("Shader Resource Binding is null");
        }
    }

    void render_command_reset(const Atomic::Graphics* graphics, RenderCommandState* state)
    {
        for (unsigned i = 0; i < Atomic::MAX_VERTEX_STREAMS; ++i)
        {
            state->vertex_buffers[i] = nullptr;
            state->vertex_offsets[i] = 0;
            s_tmp_vertex_buffers[i] = nullptr;
        }
        state->index_buffer = nullptr;

        for(uint32_t i =0; i < Atomic::MAX_RENDERTARGETS; ++i)
        {
	        state->render_targets[i] = nullptr;
			s_tmp_render_targets[i] = nullptr;
        }
        state->depth_stencil = nullptr;
        state->num_rts = 0;

        state->pipeline_hash = 0;
        state->pipeline_state = nullptr;
        state->pipeline_state_info = {};
        state->vertex_decl_hash = 0;
        state->shader_resource_binding = nullptr;

        state->pipeline_state_info.rt_formats.fill(Diligent::TEX_FORMAT_UNKNOWN);

        if(graphics->IsInitialized())
            state->viewport = Atomic::IntRect(0, 0, graphics->GetWidth(), graphics->GetHeight());
        else
            state->viewport = Atomic::IntRect::ZERO;
        state->scissor = Atomic::IntRect::ZERO;
        state->stencil_ref = 0;

        state->textures.clear();

        state->shader_program = {};
        state->dirty_state = static_cast<unsigned>(RenderCommandDirtyState::all);
    }

    void render_command_clear(const RenderCommandClearDesc& desc)
    {
        const auto driver = desc.driver;
        const auto context = driver->GetDeviceContext();

        if(desc.flags & Atomic::CLEAR_COLOR)
            context->ClearRenderTarget(desc.render_target, desc.clear_color.Data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        if(desc.flags & (Atomic::CLEAR_DEPTH | Atomic::CLEAR_STENCIL) && desc.depth_stencil)
            context->ClearDepthStencil(desc.depth_stencil,
                desc.clear_stencil_flags,
                desc.clear_depth,
                desc.clear_stencil,
                Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }

    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const float* data, uint32_t count)
    {
        buffer->SetParameter(offset, count * sizeof(float), data);
    }
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, float value)
    {
        buffer->SetParameter(offset, sizeof(float), &value);
    }
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, int value)
    {
        buffer->SetParameter(offset, sizeof(int), &value);
    }
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, bool value)
    {
        buffer->SetParameter(offset, sizeof(bool), &value);
    }
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const Color& value)
    {
	    buffer->SetParameter(offset, sizeof(float) * 4, value.Data());
    }
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const Vector2& value)
    {
        buffer->SetParameter(offset, sizeof(Vector2), value.Data());
	}
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const Vector3& value)
    {
        buffer->SetParameter(offset, sizeof(Vector3), value.Data());
	}
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const Vector4& value)
    {
        buffer->SetParameter(offset, sizeof(Vector4), value.Data());
	}
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const Matrix3x4& value)
    {
        buffer->SetParameter(offset, sizeof(Matrix3x4), &value);
    }
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const Matrix3& value)
    {
        const Matrix3x4 m(value);
        render_command_write_param(buffer, offset, m);
    }
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const Matrix4& value)
    {
        buffer->SetParameter(offset, sizeof(Matrix4), &value);
	}
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const Variant& value)
    {
	    switch(value.GetType())
	    {
	    case VAR_FLOATVECTOR:
		    {
                const auto& float_vector = value.GetFloatVector();
                render_command_write_param(buffer, offset, float_vector.Buffer(), float_vector.Size());
		    }
            break;
		case VAR_FLOAT:
		    render_command_write_param(buffer, offset, value.GetFloat());
		    break;
        case VAR_INT:
            render_command_write_param(buffer, offset, value.GetInt());
            break;
        case VAR_BOOL:
			render_command_write_param(buffer, offset, value.GetBool());
            break;
	    case VAR_COLOR:
            render_command_write_param(buffer, offset, value.GetColor());
            break;
	    case VAR_VECTOR2:
            render_command_write_param(buffer, offset, value.GetVector2());
            break;
	    case VAR_VECTOR3:
            render_command_write_param(buffer, offset, value.GetVector3());
            break;
	    case VAR_VECTOR4:
            render_command_write_param(buffer, offset, value.GetVector4());
            break;
	    case VAR_MATRIX3X4:
            render_command_write_param(buffer, offset, value.GetMatrix3x4());
            break;
	    case VAR_MATRIX3:
            render_command_write_param(buffer, offset, value.GetMatrix3());
            break;
	    case VAR_MATRIX4:
            render_command_write_param(buffer, offset, value.GetMatrix4());
			break;
	    }
    }
    void render_command_update_params(const Atomic::Graphics* graphics, RenderCommandState* state)
    {
        if (!graphics)
            return;
        if (!graphics->IsInitialized())
            return;
        if (!state->shader_program)
            return;

        Vector<ShaderParameterUpdateDesc> not_found_params;
        for(const auto& desc : state->shader_parameter_updates)
        {
            ShaderParameter parameter;
            if(!state->shader_program->GetParameter(desc.name, &parameter))
            {
				not_found_params.Push(desc);
                continue;
            }

            const auto buffer = static_cast<ConstantBuffer*>(parameter.bufferPtr_);
            if (!buffer)
                continue;
            render_command_write_param(buffer, parameter.offset_, desc.value);
        }
        // if there are not found parameters. add again to render command
        // maybe they are added later by other shader program
        if (not_found_params.Size() > 0)
            state->shader_parameter_updates = not_found_params;
        else
            state->shader_parameter_updates.Clear();

        graphics->GetImpl()->UploadBufferChanges();
    }
}
