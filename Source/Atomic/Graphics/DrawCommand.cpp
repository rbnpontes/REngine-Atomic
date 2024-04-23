#include "./DrawCommand.h"

#include "./Geometry.h"
#include "./Model.h"
#include "./Renderer.h"
#include "./Shader.h"
#include "./Texture2D.h"
#include "./TextureCube.h"
#include "./VertexBuffer.h"
#include "./IndexBuffer.h"
#include "./ShaderVariation.h"
#include "./ShaderProgram.h"
#include "./RenderTexture.h"

#include "../RHI/RenderCommand.h"
#include "../RHI/PipelineStateBuilder.h"
#include "../Core/Profiler.h"
#include "../IO/Log.h"
#include "../RHI/GraphicsState.h"
#include "../RHI/DiligentUtils.h"
#include "../RHI/ShaderParametersCache.h"

#include <GLEW/glew.h>
#if RENGINE_SSE
#include <emmintrin.h>
#endif

#define MAX_SHADER_PARAMETER_UPDATES 100

namespace REngine
{
	using namespace Atomic;

	static u8 s_num_rts = 0;

	static Diligent::IBuffer* s_index_buffer = nullptr;

	static u32 s_texture_2d_type = Texture2D::GetTypeStatic();
	static u32 s_texture_cube_type = TextureCube::GetTypeStatic();
	//static ea::queue<ShaderParameterUpdateData> s_empty_params_queue = {};
	class DrawCommandImpl : public IDrawCommand
	{
	public:
		DrawCommandImpl(Graphics* graphics, Diligent::IDeviceContext* context) :
			graphics_(graphics),
			context_(context),
			params_2_update_({}), next_param_2_update_idx_(0),
			vertex_buffers_({}),
			bind_vertex_buffers_({}),
			vertex_offsets_({}),
			shader_param_sources_({}),
			enable_clip_planes_(false),
			viewport_(IntRect::ZERO),
			dirty_flags_(static_cast<u32>(RenderCommandDirtyState::all)),
			curr_assigned_texture_flags_(0),
			curr_assigned_immutable_sa_flags_(0),
			curr_vertx_decl_checksum_(0),
			curr_vbuffer_checksum_(0),
			curr_pipeline_hash_(0),
			primitive_count_(0),
			num_batches_(0)

		{
			pipeline_info_ = new PipelineStateInfo();
			for (u32 i = 0; i < MAX_SHADER_PARAMETER_UPDATES; ++i)
				params_2_update_[i] = new ShaderParameterUpdateData();
		}

		~DrawCommandImpl() override
		{
			// TODO: clear static states too
			delete pipeline_info_;

			for (u32 i = 0; i < MAX_SHADER_PARAMETER_UPDATES; ++i)
				delete params_2_update_[i];
		}

		void Reset() override
		{
			*pipeline_info_ = PipelineStateInfo{};
			pipeline_info_->output.multi_sample = graphics_->GetMultiSample();
			
			const auto wnd_size = graphics_->GetRenderTargetDimensions();
			viewport_ = IntRect(0, 0, wnd_size.x_, wnd_size.y_);

			pipeline_state_				= nullptr;
			vertex_declaration_			= nullptr;
			shader_program_				= nullptr;
			depth_stencil_				= nullptr;
			index_buffer_				= nullptr;
			shader_resource_binding_	= nullptr;
			curr_assigned_texture_flags_ = curr_assigned_immutable_sa_flags_ = 0;

			index_type_ = VT_UNDEFINED;

			shader_param_sources_.fill(M_MAX_UNSIGNED);

			render_targets_.fill(nullptr);
			for (u8 i = 0; i < MAX_TEXTURE_UNITS; ++i)
				ResetTexture(static_cast<TextureUnit>(i));
			vertex_buffers_.fill(nullptr);
			vertex_offsets_.fill(0);

			enable_clip_planes_ = false;
			if (graphics_->GetBackend() == GraphicsBackend::OpenGL)
				glDisable(GL_CLIP_PLANE0);

			clip_plane_ = Vector4::ZERO;
			curr_pipeline_hash_			= 
			curr_vbuffer_checksum_		= 
			curr_vertx_decl_checksum_	= 0u;

			next_param_2_update_idx_ = 0;
			textures_in_use_ = 0;
			num_batches_ = 0;
			primitive_count_ = 0;

			dirty_flags_ = static_cast<u32>(RenderCommandDirtyState::all);
		}
		void Clear(const DrawCommandClearDesc& desc) override
		{
			ATOMIC_PROFILE(IDrawCommand::Clear);
			const auto rt_size = graphics_->GetRenderTargetDimensions();

			if(!viewport_.left_ && !viewport_.top_ && viewport_.right_ == rt_size.x_ && viewport_.bottom_ == rt_size.y_)
			{
				const auto old_depth_write = pipeline_info_->depth_write_enabled;
				const auto old_dirty_flags = dirty_flags_;

				pipeline_info_->depth_write_enabled = true;
				PrepareClear();

				pipeline_info_->depth_write_enabled = old_depth_write;
				// if previous dirty flags does not change pipeline. then we must remove pipeline from dirty flags
				if(old_dirty_flags & static_cast<u32>(RenderCommandDirtyState::pipeline) == 0)
					dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::pipeline);

				ClearByHardware(desc);
				return;
			}

			PrepareClear();
			ClearBySoftware(desc);
		}
		void Draw(const DrawCommandDrawDesc& desc) override
		{
			ATOMIC_PROFILE(IDrawCommand::Draw);

			PrepareDraw();

			if (!pipeline_state_)
				return;

			if(index_buffer_)
			{
				context_->DrawIndexed({
					Min(desc.index_count, index_buffer_->GetIndexCount()),
					index_type_,
					DRAW_FLAG_NONE,
					1,
					desc.index_start,
					desc.base_vertex_index
				});
			}
			else
			{
				context_->Draw({
					desc.vertex_count,
					DRAW_FLAG_NONE,
					1,
					desc.vertex_start
				});
			}

#if ATOMIC_DEBUG
			u32 primitive_count;
			utils_get_primitive_type(desc.vertex_count, pipeline_info_->primitive_type, &primitive_count);
			primitive_count_ += primitive_count;
			++num_batches_;
#endif
		}
		void Draw(const DrawCommandInstancedDrawDesc& desc) override
		{
			ATOMIC_PROFILE(IDrawCommand::Draw);

			PrepareDraw();

			context_->DrawIndexed({
				desc.index_count,
				index_type_,
				DRAW_FLAG_NONE,
				desc.instance_count,
				desc.index_start,
				desc.base_vertex_index
			});

#if ATOMIC_DEBUG
			u32 primitive_count;
			utils_get_primitive_type(desc.vertex_count, pipeline_info_->primitive_type, &primitive_count);
			primitive_count_ += primitive_count * desc.instance_count;
			++num_batches_;
#endif
		}
		void SetVertexBuffer(VertexBuffer* buffer) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetVertexBuffer);
			if (vertex_buffers_[0].get() == buffer)
				return;

			vertex_offsets_[0] = 0;
			if (!buffer)
			{
				vertex_buffers_[0] = nullptr;
				num_vertex_buffers_ = curr_vertx_decl_checksum_ = curr_vertx_decl_checksum_ = 0;
			}
			else
			{
				buffer->AddRef();
				vertex_buffers_[0].reset(buffer, ea::EngineRefCounterDeleter<VertexBuffer>());
				bind_vertex_buffers_[0] = buffer->GetGPUObject().Cast<IBuffer>(IID_Buffer);

				num_vertex_buffers_ = 1;
				curr_vbuffer_checksum_ = 16777619;
				curr_vbuffer_checksum_ ^= reinterpret_cast<u32>(bind_vertex_buffers_[0]) * 16777619;
				curr_vertx_decl_checksum_ = buffer->GetBufferHash(0);
			}

			if(curr_vbuffer_checksum_ != last_vbuffer_checksum_)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::vertex_buffer);
			last_vbuffer_checksum_ = curr_vbuffer_checksum_;
		}
		void SetVertexBuffers(const PODVector<VertexBuffer*> buffers, u32 instance_offset) override
		{
			SetVertexBuffers(buffers.Buffer(), buffers.Size(), instance_offset);
		}
		void SetVertexBuffers(const ea::vector<VertexBuffer*>& buffers, u32 instance_offset) override
		{
			SetVertexBuffers(const_cast<VertexBuffer**>(buffers.data()), buffers.size(), instance_offset);
		}
		void SetIndexBuffer(IndexBuffer* buffer) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetIndexBuffer);
			if(index_buffer_.get() == buffer)
				return;

			if(buffer)
			{
				buffer->AddRef();
				index_buffer_.reset(buffer, ea::EngineRefCounterDeleter<IndexBuffer>());
				index_type_ = buffer->GetIndexSize() == sizeof(u32) ? VT_UINT32 : VT_UINT16;
			}
			else
				index_buffer_.reset();

			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::index_buffer);
		}
		void SetShaders(const DrawCommandShadersDesc& desc) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetShaders);
			// TODO: add support for other shaders
			static ShaderVariation* s_shaders[MAX_SHADER_TYPES] = {};
			s_shaders[VS] = desc.vs;
			s_shaders[PS] = desc.ps;

			static ShaderVariation* s_pipeline_shaders[MAX_SHADER_TYPES] = {};
			s_pipeline_shaders[VS] = pipeline_info_->vs_shader;
			s_pipeline_shaders[PS] = pipeline_info_->ps_shader;

			bool changed = false;
			for(u8 i = 0; i < MAX_SHADER_TYPES; ++i)
			{
				auto shader = s_shaders[i];
				const auto pipeline_shader = s_pipeline_shaders[i];
				if(shader == pipeline_shader)
					continue;

				if(enable_clip_planes_)
				{
					shader = shader->GetOwner()->GetVariation(
						static_cast<ShaderType>(i),
						shader->GetDefinesClipPlane()
					);
				}

				if (shader == pipeline_shader)
					continue;

				if(i == VS)
					dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::vertex_decl) | static_cast<u32>(RenderCommandDirtyState::vertex_buffer);

				// Build shader if is necessary
				if(shader && !shader->GetGPUObject())
				{
					if (!shader->GetCompilerOutput().Empty())
						shader = nullptr;
					else if(!shader->Create())
					{
						ATOMIC_LOGERROR("Failed to create shader: " + shader->GetName());
						shader = nullptr;
					}
				}

				s_shaders[i] = shader;
				changed = true;
			}

			if (!changed)
				return;

			pipeline_info_->vs_shader = s_shaders[VS];
			pipeline_info_->ps_shader = s_shaders[PS];
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);

			if(s_shaders[VS] && s_shaders[PS])
			{
				const ShaderProgramQuery query{
					s_shaders[VS],
					s_shaders[PS]
				};
				shader_program_ = GetOrCreateShaderProgram(query);

				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::shader_program);
			}
			else
			{
				shader_program_ = nullptr;
				dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::shader_program);
			}

			if (enable_clip_planes_)
				SetShaderParameter(VSP_CLIPPLANE, clip_plane_);
		}
		void SetShaderParameter(StringHash param, const float* data, u32 count) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetShaderParameter);
			ATOMIC_PROFILE_MSG("FloatVector");
			const auto shader_program = shader_program_;

			ShaderParameter* parameter = nullptr;
			if(shader_parameters_cache_get(param.Value(), &parameter))
			{
				const auto cbuffer = static_cast<ConstantBuffer*>(parameter->bufferPtr_);
				cbuffer->SetParameter(parameter->offset_, sizeof(float) * count, data);
				return;
			}

			const auto idx = Min(next_param_2_update_idx_++, MAX_SHADER_PARAMETER_UPDATES);
			// If parameters is exceeded we must skip writing
			if(idx == MAX_SHADER_PARAMETER_UPDATES)
			{
				ATOMIC_LOGWARNING("Exceeded number of writing at shader parameters");
				return;
			}

			const auto param_data = params_2_update_[idx];
			param_data->value = FloatVector(data, count);
			param_data->hash = param.Value();
		}
		void SetShaderParameter(StringHash param, const FloatVector& value) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetShaderParameter);
			ATOMIC_PROFILE_MSG("FloatVector");
			const auto shader_program = shader_program_;

			ShaderParameter* parameter = nullptr;
			if(shader_parameters_cache_get(param.Value(), &parameter))
			{
				const auto cbuffer = static_cast<ConstantBuffer*>(parameter->bufferPtr_);
				cbuffer->SetParameter(parameter->offset_, sizeof(float) * value.Size(), value.Buffer());
				return;
			}

			const auto idx = Min(next_param_2_update_idx_++, MAX_SHADER_PARAMETER_UPDATES);
			// If parameters is exceeded we must skip writing
			if(idx == MAX_SHADER_PARAMETER_UPDATES)
			{
				ATOMIC_LOGWARNING("Exceeded number of writing at shader parameters");
				return;
			}

			const auto param_data = params_2_update_[idx];
			param_data->value = value;
			param_data->hash = param.Value();
		}
		void SetShaderParameter(StringHash param, float value) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetShaderParameter);
			ATOMIC_PROFILE_MSG("float");
			const auto shader_program = shader_program_;

			ShaderParameter* parameter = nullptr;
			if(shader_parameters_cache_get(param.Value(), &parameter))
			{
				const auto cbuffer = static_cast<ConstantBuffer*>(parameter->bufferPtr_);
				cbuffer->SetParameter(parameter->offset_, sizeof(float), &value);
				return;
			}

			const auto idx = Min(next_param_2_update_idx_++, MAX_SHADER_PARAMETER_UPDATES);
			// If parameters is exceeded we must skip writing
			if(idx == MAX_SHADER_PARAMETER_UPDATES)
			{
				ATOMIC_LOGWARNING("Exceeded number of writing at shader parameters");
				return;
			}

			const auto param_data = params_2_update_[idx];
			param_data->value = value;
			param_data->hash = param.Value();
		}
		void SetShaderParameter(StringHash param, int value) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetShaderParameter);
			ATOMIC_PROFILE_MSG("int");
			const auto shader_program = shader_program_;

			ShaderParameter* parameter = nullptr;
			if(shader_parameters_cache_get(param.Value(), &parameter))
			{
				const auto cbuffer = static_cast<ConstantBuffer*>(parameter->bufferPtr_);
				cbuffer->SetParameter(parameter->offset_, sizeof(int), &value);
				return;
			}

			const auto idx = Min(next_param_2_update_idx_++, MAX_SHADER_PARAMETER_UPDATES);
			// If parameters is exceeded we must skip writing
			if(idx == MAX_SHADER_PARAMETER_UPDATES)
			{
				ATOMIC_LOGWARNING("Exceeded number of writing at shader parameters");
				return;
			}

			const auto param_data = params_2_update_[idx];
			param_data->value = value;
			param_data->hash = param.Value();
		}
		void SetShaderParameter(StringHash param, bool value) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetShaderParameter);
			ATOMIC_PROFILE_MSG("bool");
			const auto shader_program = shader_program_;

			ShaderParameter* parameter = nullptr;
			if(shader_parameters_cache_get(param.Value(), &parameter))
			{
				const auto cbuffer = static_cast<ConstantBuffer*>(parameter->bufferPtr_);
				cbuffer->SetParameter(parameter->offset_, sizeof(bool), &value);
				return;
			}

			const auto idx = Min(next_param_2_update_idx_++, MAX_SHADER_PARAMETER_UPDATES);
			// If parameters is exceeded we must skip writing
			if(idx == MAX_SHADER_PARAMETER_UPDATES)
			{
				ATOMIC_LOGWARNING("Exceeded number of writing at shader parameters");
				return;
			}

			const auto param_data = params_2_update_[idx];
			param_data->value = value;
			param_data->hash = param.Value();
		}
		void SetShaderParameter(StringHash param, const Color& value) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetShaderParameter);
			ATOMIC_PROFILE_MSG("Color");
			const auto shader_program = shader_program_;

			ShaderParameter* parameter = nullptr;
			if(shader_parameters_cache_get(param.Value(), &parameter))
			{
				const auto cbuffer = static_cast<ConstantBuffer*>(parameter->bufferPtr_);
				cbuffer->SetParameter(parameter->offset_, sizeof(Color), &value);
				return;
			}

			const auto idx = Min(next_param_2_update_idx_++, MAX_SHADER_PARAMETER_UPDATES);
			// If parameters is exceeded we must skip writing
			if(idx == MAX_SHADER_PARAMETER_UPDATES)
			{
				ATOMIC_LOGWARNING("Exceeded number of writing at shader parameters");
				return;
			}

			const auto param_data = params_2_update_[idx];
			param_data->value = value;
			param_data->hash = param.Value();
		}
		void SetShaderParameter(StringHash param, const Atomic::Vector2& value) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetShaderParameter);
			ATOMIC_PROFILE_MSG("Vector2");
			const auto shader_program = shader_program_;

			ShaderParameter* parameter = nullptr;
			if(shader_parameters_cache_get(param.Value(), &parameter))
			{
				const auto cbuffer = static_cast<ConstantBuffer*>(parameter->bufferPtr_);
				cbuffer->SetParameter(parameter->offset_, sizeof(Vector2), &value);
				return;
			}

			const auto idx = Min(next_param_2_update_idx_++, MAX_SHADER_PARAMETER_UPDATES);
			// If parameters is exceeded we must skip writing
			if(idx == MAX_SHADER_PARAMETER_UPDATES)
			{
				ATOMIC_LOGWARNING("Exceeded number of writing at shader parameters");
				return;
			}

			const auto param_data = params_2_update_[idx];
			param_data->value = value;
			param_data->hash = param.Value();
		}
		void SetShaderParameter(StringHash param, const Atomic::Vector3& value) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetShaderParameter);
			ATOMIC_PROFILE_MSG("Vector3");
			const auto shader_program = shader_program_;

			ShaderParameter* parameter = nullptr;
			if(shader_parameters_cache_get(param.Value(), &parameter))
			{
				const auto cbuffer = static_cast<ConstantBuffer*>(parameter->bufferPtr_);
				cbuffer->SetParameter(parameter->offset_, sizeof(Vector3), &value);
				return;
			}

			const auto idx = Min(next_param_2_update_idx_++, MAX_SHADER_PARAMETER_UPDATES);
			// If parameters is exceeded we must skip writing
			if(idx == MAX_SHADER_PARAMETER_UPDATES)
			{
				ATOMIC_LOGWARNING("Exceeded number of writing at shader parameters");
				return;
			}

			const auto param_data = params_2_update_[idx];
			param_data->value = value;
			param_data->hash = param.Value();
		}
		void SetShaderParameter(StringHash param, const Atomic::Vector4& value) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetShaderParameter);
			ATOMIC_PROFILE_MSG("Vector4");
			const auto shader_program = shader_program_;

			ShaderParameter* parameter = nullptr;
			if(shader_parameters_cache_get(param.Value(), &parameter))
			{
				const auto cbuffer = static_cast<ConstantBuffer*>(parameter->bufferPtr_);
				cbuffer->SetParameter(parameter->offset_, sizeof(Vector4), &value);
				return;
			}

			const auto idx = Min(next_param_2_update_idx_++, MAX_SHADER_PARAMETER_UPDATES);
			// If parameters is exceeded we must skip writing
			if(idx == MAX_SHADER_PARAMETER_UPDATES)
			{
				ATOMIC_LOGWARNING("Exceeded number of writing at shader parameters");
				return;
			}

			const auto param_data = params_2_update_[idx];
			param_data->value = value;
			param_data->hash = param.Value();
		}
		void SetShaderParameter(StringHash param, const IntVector2& value) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetShaderParameter);
			ATOMIC_PROFILE_MSG("IntVector2");
			const auto shader_program = shader_program_;

			ShaderParameter* parameter = nullptr;
			if(shader_parameters_cache_get(param.Value(), &parameter))
			{
				const auto cbuffer = static_cast<ConstantBuffer*>(parameter->bufferPtr_);
				cbuffer->SetParameter(parameter->offset_, sizeof(IntVector2), &value);
				return;
			}

			const auto idx = Min(next_param_2_update_idx_++, MAX_SHADER_PARAMETER_UPDATES);
			// If parameters is exceeded we must skip writing
			if(idx == MAX_SHADER_PARAMETER_UPDATES)
			{
				ATOMIC_LOGWARNING("Exceeded number of writing at shader parameters");
				return;
			}

			const auto param_data = params_2_update_[idx];
			param_data->value = value;
			param_data->hash = param.Value();
		}
		void SetShaderParameter(StringHash param, const IntVector3& value) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetShaderParameter);
			ATOMIC_PROFILE_MSG("IntVector3");
			const auto shader_program = shader_program_;

			ShaderParameter* parameter = nullptr;
			if(shader_parameters_cache_get(param.Value(), &parameter))
			{
				const auto cbuffer = static_cast<ConstantBuffer*>(parameter->bufferPtr_);
				cbuffer->SetParameter(parameter->offset_, sizeof(IntVector3), &value);
				return;
			}

			const auto idx = Min(next_param_2_update_idx_++, MAX_SHADER_PARAMETER_UPDATES);
			// If parameters is exceeded we must skip writing
			if(idx == MAX_SHADER_PARAMETER_UPDATES)
			{
				ATOMIC_LOGWARNING("Exceeded number of writing at shader parameters");
				return;
			}

			const auto param_data = params_2_update_[idx];
			param_data->value = value;
			param_data->hash = param.Value();
		}
		void SetShaderParameter(StringHash param, const Matrix3& value) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetShaderParameter);
			ATOMIC_PROFILE_MSG("Matrix3");

			const auto shader_program = shader_program_;

			ShaderParameter* parameter = nullptr;
			if(shader_parameters_cache_get(param.Value(), &parameter))
			{
				const auto cbuffer = static_cast<ConstantBuffer*>(parameter->bufferPtr_);
				const auto backend = graphics_->GetImpl()->GetBackend();
				if(backend == GraphicsBackend::Vulkan || backend == GraphicsBackend::OpenGL)
				{
#if RENGINE_SSE
					float* data = static_cast<float*>(cbuffer->GetWriteBuffer(parameter->offset_));
					cbuffer->MakeDirty();
					// I just need to copy matrix in a faster way
					__m128 row = _mm_set_ps(0.0f, value.m02_, value.m01_, value.m00_);
					_mm_store_ps(data, row);

					data += 4;

					row = _mm_set_ps(0.0f, value.m12_, value.m11_, value.m10_);
					_mm_store_ps(data, row);

					data += 4;

					row = _mm_set_ps(0.0f, value.m22_, value.m21_, value.m20_);
					_mm_store_ps(data, row);
#else
					const auto data = static_cast<const float*>(static_cast<const void*>(&value));
					size_t offset = 0;
					size_t matrix_offset = 0;
					for(u32 j =0; j < 3; ++j)
					{
						for(u32 i =0; i < 3; ++i)
						{
							cbuffer->SetParameter(parameter->offset_ + offset, sizeof(float), data + matrix_offset);
							offset += sizeof(float);
							++matrix_offset;
						}
						offset += sizeof(float);
					}
#endif
				}
				else
					cbuffer->SetParameter(parameter->offset_, sizeof(Matrix3), &value);
				return;
			}

			const auto idx = Min(next_param_2_update_idx_++, MAX_SHADER_PARAMETER_UPDATES);
			// If parameters is exceeded we must skip writing
			if(idx == MAX_SHADER_PARAMETER_UPDATES)
			{
				ATOMIC_LOGWARNING("Exceeded number of writing at shader parameters");
				return;
			}

			const auto param_data = params_2_update_[idx];
			param_data->value = value;
			param_data->hash = param.Value();
		}
		void SetShaderParameter(StringHash param, const Matrix3x4& value) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetShaderParameter);
			ATOMIC_PROFILE_MSG("Matrix3x4");

			const auto shader_program = shader_program_;

			ShaderParameter* parameter = nullptr;
			if(shader_parameters_cache_get(param.Value(), &parameter))
			{
				static constexpr float s_last_matrix_row[] = {0.0f, 0.0f, 0.0f, 1.0f};
				const auto cbuffer = static_cast<ConstantBuffer*>(parameter->bufferPtr_);
				// Constant Buffer on shader expect a Matrix4x4
				// Convert an Matrix3x4 to Matrix4 can be expensive, in this case
				// We will copy last row to buffer instead of create a Matrix4x4
				cbuffer->SetParameter(parameter->offset_, sizeof(Matrix3x4), &value);
				cbuffer->SetParameter(parameter->offset_ + sizeof(Matrix3x4), sizeof(float) * _countof(s_last_matrix_row), s_last_matrix_row);
				return;
			}

			const auto idx = Min(next_param_2_update_idx_++, MAX_SHADER_PARAMETER_UPDATES);
			// If parameters is exceeded we must skip writing
			if(idx == MAX_SHADER_PARAMETER_UPDATES)
			{
				ATOMIC_LOGWARNING("Exceeded number of writing at shader parameters");
				return;
			}

			const auto param_data = params_2_update_[idx];
			param_data->value = value;
			param_data->hash = param.Value();
		}
		void SetShaderParameter(StringHash param, const Matrix4& value) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetShaderParameter);
			ATOMIC_PROFILE_MSG("Matrix4");
			const auto shader_program = shader_program_;

			ShaderParameter* parameter = nullptr;
			if(shader_parameters_cache_get(param.Value(), &parameter))
			{
				const auto cbuffer = static_cast<ConstantBuffer*>(parameter->bufferPtr_);
				cbuffer->SetParameter(parameter->offset_, sizeof(Matrix4), &value);
				return;
			}

			const auto idx = Min(next_param_2_update_idx_++, MAX_SHADER_PARAMETER_UPDATES);
			// If parameters is exceeded we must skip writing
			if(idx == MAX_SHADER_PARAMETER_UPDATES)
			{
				ATOMIC_LOGWARNING("Exceeded number of writing at shader parameters");
				return;
			}

			const auto param_data = params_2_update_[idx];
			param_data->value = value;
			param_data->hash = param.Value();
		}
		void SetShaderParameter(StringHash param, const Variant& value) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetShaderParameter);
			ATOMIC_PROFILE_MSG("Variant");
			const auto shader_program = shader_program_;

			ShaderParameter* parameter = nullptr;
			if(shader_parameters_cache_get(param.Value(), &parameter))
			{
				const auto cbuffer = static_cast<ConstantBuffer*>(parameter->bufferPtr_);
				render_command_write_param(cbuffer, parameter->offset_, &value);
				return;
			}

			const auto idx = Min(next_param_2_update_idx_++, MAX_SHADER_PARAMETER_UPDATES);
			// If parameters is exceeded we must skip writing
			if(idx == MAX_SHADER_PARAMETER_UPDATES)
			{
				ATOMIC_LOGWARNING("Exceeded number of writing at shader parameters");
				return;
			}

			const auto param_data = params_2_update_[idx];
			param_data->value = value;
			param_data->hash = param.Value();
		}
		bool HasShaderParameter(StringHash param) override
		{
			return shader_parameters_cache_has(param.Value());
		}
		bool NeedShaderGroupUpdate(ShaderParameterGroup group, const void* source) override
		{
			const auto src = shader_param_sources_[group];
			const auto target_src = reinterpret_cast<u32>(source);
			if(src == M_MAX_UNSIGNED || src != target_src)
			{
				shader_param_sources_[group] = target_src;
				return true;
			}
			return false;
		}
		void ClearShaderParameterSource(ShaderParameterGroup group) override
		{
			shader_param_sources_[group] = M_MAX_UNSIGNED;
		}
		void SetTexture(TextureUnit unit, RenderSurface* surface) override
		{
			return SetTexture(unit, surface ? surface->GetParentTexture() : static_cast<Texture*>(nullptr));
		}
		void SetTexture(TextureUnit unit, Texture* texture) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetTexture);
			if (unit >= MAX_TEXTURE_UNITS)
				return;

			if(texture)
			{
				const auto parent_texture = render_targets_[0] ? render_targets_[0]->GetParentTexture() : nullptr;
				const auto multi_sample = texture->GetMultiSample();
				const auto auto_resolve = texture->GetAutoResolve();
				const auto is_resolved_dirty = texture->IsResolveDirty();
				const auto levels_dirty = texture->GetLevelsDirty();
				const auto params_dirty = texture->GetParametersDirty();

				if (parent_texture == texture)
					texture = texture->GetBackupTexture();
				else if(multi_sample > 1 && auto_resolve && is_resolved_dirty)
				{
					const auto texture_type = texture->GetType().Value();
					if(s_texture_2d_type == texture_type)
						ResolveTexture(static_cast<Texture2D*>(texture));
					else if(s_texture_cube_type == texture_type)
						ResolveTexture(static_cast<TextureCube*>(texture));
				}

				if (texture->GetLevelsDirty())
					texture->RegenerateLevels();
				if(params_dirty)
				{
					texture->UpdateParameters();
					textures_[unit].texture = {};
				}

				curr_assigned_texture_flags_ |= 1 << unit;
			}
			else
				curr_assigned_texture_flags_ ^= 1 << unit;


			if (texture == textures_[unit].owner.get())
				return;

			const auto view = texture ? texture->GetShaderResourceView() : RefCntAutoPtr<ITextureView>();
			textures_[unit] = {
				nullptr,
				StringHash::ZERO,
				unit,
				view,
				ea::MakeShared(texture)
			};

			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::textures);
		}
		void SetTexture(TextureUnit unit, RenderTexture* texture) override
		{
			SetTexture(unit, texture->GetBackBuffer().get());
		}
		bool HasTexture(TextureUnit unit) override
		{
			if (unit >= MAX_TEXTURE_UNITS || !shader_program_)
				return false;
			return shader_program_->GetSampler(unit) != nullptr;
		}
		void SetRenderTarget(u8 index, RenderSurface* surface) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetRenderTarget);
			if (index >= MAX_RENDERTARGETS)
				return;

			if (render_targets_[index].get() == surface)
				return;

			if(surface)
			{
				surface->AddRef();
				render_targets_[index].reset(surface, ea::EngineRefCounterDeleter<RenderSurface>());
			}
			else
			{
				render_targets_[index].reset();
				bind_rts_[index] = nullptr;
			}

			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::render_targets);

			if (!surface)
				return;

			Texture* parent_texture = surface->GetParentTexture();
			for(u8 i =0; i < MAX_TEXTURE_UNITS; ++i)
			{
				if(textures_[i].owner.get() == parent_texture)
					SetTexture(static_cast<TextureUnit>(i), textures_[i].owner->GetBackupTexture());
			}

			if(parent_texture->GetMultiSample() > 1 && parent_texture->GetAutoResolve())
			{
				parent_texture->SetResolveDirty(true);
				surface->SetResolveDirty(true);
			}

			if (parent_texture->GetLevels() > 1)
				parent_texture->SetLevelsDirty();
		}
		void SetRenderTarget(u8 index, Texture2D* texture) override
		{
			if (texture)
				SetRenderTarget(index, texture->GetRenderSurface());
			else
				SetRenderTarget(index, static_cast<RenderSurface*>(nullptr));
		}
		void SetRenderTarget(u8 index, RenderTexture* texture) override
		{
			SetDepthStencil(texture->GetDepthStencil().get());
			SetRenderTarget(index, texture->GetBackBuffer().get());
		}
		void SetDepthStencil(RenderSurface* surface) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetDepthStencil);
			if(depth_stencil_.get() == surface)
				return;

			if(surface)
			{
				surface->AddRef();
				depth_stencil_.reset(surface, ea::EngineRefCounterDeleter<RenderSurface>());
			}
			else
				depth_stencil_.reset();
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::render_targets);
		}
		void SetDepthStencil(Texture2D* texture) override
		{
			if (texture)
				SetDepthStencil(texture->GetRenderSurface());
			else
				SetDepthStencil(static_cast<RenderSurface*>(nullptr));
		}
		void ResetRenderTarget(u8 index) override
		{
			SetRenderTarget(index, static_cast<RenderSurface*>(nullptr));
		}
		void ResetRenderTargets() override
		{
			for (u8 i = 0; i < MAX_RENDERTARGETS; ++i)
				ResetRenderTarget(i);
		}
		void ResetDepthStencil() override
		{
			SetDepthStencil(static_cast<RenderSurface*>(nullptr));
		}
		void ResetTexture(TextureUnit unit) override
		{
			if (unit >= MAX_TEXTURE_UNITS)
				return;

			if (textures_[unit].unit == MAX_TEXTURE_UNITS)
				return;

			textures_[unit].unit = MAX_TEXTURE_UNITS;
			textures_[unit].texture = nullptr;
			textures_[unit].owner.reset();
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::textures);
		}
		void SetPrimitiveType(PrimitiveType type) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetPrimitiveType);
			if(pipeline_info_->primitive_type != type)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->primitive_type = type;
		}
		void SetViewport(const IntRect& viewport) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetViewport);
			const IntVector2 size = graphics_->GetRenderTargetDimensions();
			IntRect rect_cpy = viewport;

			if (rect_cpy.right_ <= rect_cpy.left_)
				rect_cpy.right_ = rect_cpy.left_ + 1;
			if (rect_cpy.bottom_ <= rect_cpy.top_)
				rect_cpy.bottom_ = rect_cpy.top_ + 1;
			rect_cpy.left_	 = Clamp(rect_cpy.left_, 0, size.x_);
			rect_cpy.top_	 = Clamp(rect_cpy.top_, 0, size.y_);
			rect_cpy.right_	 = Clamp(rect_cpy.right_, 0, size.x_);
			rect_cpy.bottom_ = Clamp(rect_cpy.bottom_, 0, size.y_);

			if(viewport_ == rect_cpy)
				return;

			viewport_ = rect_cpy;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::viewport);

			SetScissorTest(false);
		}
		void SetBlendMode(BlendMode mode, bool alpha_to_coverage) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetBlendMode);
			if(pipeline_info_->blend_mode != mode)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			if(pipeline_info_->alpha_to_coverage_enabled != alpha_to_coverage)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->blend_mode = mode;
			pipeline_info_->alpha_to_coverage_enabled = alpha_to_coverage;
		}
		void SetColorWrite(bool enable) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetColorWrite);
			if(pipeline_info_->color_write_enabled == enable)
				return;
			pipeline_info_->color_write_enabled = enable;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
		}
		void SetCullMode(CullMode mode) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetCullMode);
			if(pipeline_info_->cull_mode == mode)
				return;
			pipeline_info_->cull_mode = mode;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
		}
		void SetDepthBias(float constant_bias, float slope_scaled_bias) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetDepthBias);
			if(pipeline_info_->constant_depth_bias == constant_bias && pipeline_info_->slope_scaled_depth_bias == slope_scaled_bias)
				return;
			pipeline_info_->constant_depth_bias = constant_bias;
			pipeline_info_->slope_scaled_depth_bias = slope_scaled_bias;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
		}
		void SetDepthTest(CompareMode mode) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetDepthTest);
			if(pipeline_info_->depth_cmp_function == mode)
				return;
			pipeline_info_->depth_cmp_function = mode;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
		}
		void SetDepthWrite(bool enable) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetDepthWrite);
			if(pipeline_info_->depth_write_enabled == enable)
				return;
			pipeline_info_->depth_write_enabled = enable;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
		}
		void SetFillMode(FillMode mode) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetFillMode);
			if(pipeline_info_->fill_mode == mode)
				return;
			pipeline_info_->fill_mode = mode;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
		}
		void SetLineAntiAlias(bool enable) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetLineAntiAlias);
			if(pipeline_info_->line_anti_alias == enable)
				return;
			pipeline_info_->line_anti_alias = enable;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
		}
		void SetScissorTest(bool enable, const IntRect& rect) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetScissorTest);
			const IntVector2 rt_size = graphics_->GetRenderTargetDimensions();
			const IntVector2 view_pos(viewport_.left_, viewport_.top_);

			if(enable)
			{
				IntRect int_rect;
				int_rect.left_	 = Clamp(rect.left_	+ view_pos.x_, 0, rt_size.x_ - 1);
				int_rect.top_	 = Clamp(rect.top_		+ view_pos.y_, 0, rt_size.y_ - 1);
				int_rect.right_  = Clamp(rect.right_	+ view_pos.x_, 0, rt_size.x_);
				int_rect.bottom_ = Clamp(rect.bottom_	+ view_pos.y_, 0, rt_size.y_);

				if(int_rect.right_ == int_rect.left_)
					int_rect.right_++;
				if(int_rect.bottom_ == int_rect.top_)
					int_rect.bottom_++;

				if(int_rect.right_ < int_rect.left_ || int_rect.bottom_ < int_rect.top_)
					enable = false;

				if(enable && int_rect != scissor_)
				{
					scissor_ = int_rect;
					dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::scissor);
				}
			}

			if(pipeline_info_->scissor_test_enabled != enable)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->scissor_test_enabled = enable;
		}
		void SetScissorTest(bool enable, const Atomic::Rect& rect = Atomic::Rect::FULL, bool border_inclusive = true) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetScissorTest);
			// During some light rendering loops, a full rect is toggled on/off repeatedly.
			// Disable scissor in that case to reduce state changes
			if(rect.min_.x_ <= 0.0f && rect.min_.y_ <= 0.0f && rect.max_.x_ >= 1.0f && rect.max_.y_ >= 1.0f)
				enable = false;

			if(enable)
			{
				const IntVector2 rt_size = graphics_->GetRenderTargetDimensions();
				const IntVector2 view_size(viewport_.Size());
				const IntVector2 view_pos(viewport_.left_, viewport_.top_);
				const int expand = border_inclusive ? 1 : 0;
				IntRect int_rect;

				int_rect.left_	 = Clamp(static_cast<i32>((rect.min_.x_ + 1.0f) * 0.5f * view_size.x_) + view_pos.x_, 0, rt_size.x_ - 1);
				int_rect.top_	 = Clamp(static_cast<i32>((-rect.max_.y_ + 1.0f) * 0.5f * view_size.y_) + view_pos.y_, 0, rt_size.y_ - 1);
				int_rect.right_	 = Clamp(static_cast<i32>((rect.max_.x_ + 1.0f) * 0.5f * view_size.x_) + view_pos.x_ + expand, 0, rt_size.x_);
				int_rect.bottom_ = Clamp(static_cast<i32>((-rect.min_.y_ + 1.0f) * 0.5f * view_size.y_) + view_pos.y_ + expand, 0, rt_size.y_);

				if (int_rect.right_ == int_rect.left_)
					int_rect.right_++;
				if(int_rect.bottom_ == int_rect.top_)
					int_rect.bottom_++;

				if(int_rect.right_ < int_rect.left_ || int_rect.bottom_ < int_rect.top_)
					enable = false;

				if(enable && int_rect != scissor_)
				{
					scissor_ = int_rect;
					dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::scissor);
				}
			}

			if(pipeline_info_->scissor_test_enabled != enable)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);

			pipeline_info_->scissor_test_enabled = enable;
		}
		void SetStencilTest(const DrawCommandStencilTestDesc& desc) override
		{
			ATOMIC_PROFILE(IDrawCommand::SetStencilTest);
			if(desc.enable != pipeline_info_->stencil_test_enabled)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->stencil_test_enabled = desc.enable;

			if(!desc.enable)
				return;

			if(desc.mode != pipeline_info_->stencil_cmp_function)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->stencil_cmp_function = desc.mode;

			if(desc.pass != pipeline_info_->stencil_op_on_passed)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->stencil_op_on_passed = desc.pass;

			if(desc.fail != pipeline_info_->stencil_op_on_stencil_failed)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->stencil_op_on_stencil_failed = desc.fail;

			if(desc.depth_fail != pipeline_info_->stencil_op_depth_failed)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->stencil_op_depth_failed = desc.depth_fail;

			if(desc.compare_mask != pipeline_info_->stencil_cmp_mask)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->stencil_cmp_mask = static_cast<u8>(desc.compare_mask);

			if(desc.write_mask != pipeline_info_->stencil_write_mask)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->stencil_write_mask = static_cast<u8>(desc.write_mask);

			if(desc.stencil_ref != stencil_ref_)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			stencil_ref_ = static_cast<u8>(desc.stencil_ref);
		}
		void SetClipPlane(const DrawCommandClipPlaneDesc& desc) override
		{
			if(desc.enable != enable_clip_planes_ && graphics_->GetBackend() == GraphicsBackend::OpenGL)
			{
				if (desc.enable)
					glEnable(GL_CLIP_PLANE0);
				else
					glDisable(GL_CLIP_PLANE0);
			}

			enable_clip_planes_ = desc.enable;
			if(!enable_clip_planes_)
				return;

			const Matrix4 view_proj = desc.projection * desc.view;
			clip_plane_ = desc.clip_plane.Transformed(view_proj).ToVector4();
			SetShaderParameter(VSP_CLIPPLANE, clip_plane_);
		}
		bool ResolveTexture(Texture2D* dest) override
		{
			throw std::exception("Not implemented");
		}
		bool ResolveTexture(Texture2D* dest, const IntRect& viewport) override
		{
			ATOMIC_PROFILE(IDrawCommand::ResolveTexture);
			if(!dest || !dest->GetRenderSurface())
				return false;

			const auto rt_size = graphics_->GetRenderTargetDimensions();
			IntRect vp_copy = viewport;
			if (vp_copy.right_ <= vp_copy.left_)
				vp_copy.right_ = vp_copy.left_ + 1;
			if (vp_copy.bottom_ <= vp_copy.top_)
				vp_copy.bottom_ = vp_copy.top_ + 1;

			Diligent::Box src_box;
			src_box.MinX = Clamp(vp_copy.left_, 0, rt_size.x_);
			src_box.MinY = Clamp(vp_copy.top_, 0, rt_size.y_);
			src_box.MaxX = Clamp(vp_copy.right_, 0, rt_size.x_);
			src_box.MaxY = Clamp(vp_copy.bottom_, 0, rt_size.y_);
			src_box.MinZ = 0;
			src_box.MaxZ = 1;

			// TODO: enable this when MSAA is available
			//bool resolve = multiSample_ > 1;
			bool resolve = false;
			auto source = graphics_->GetImpl()->GetSwapChain()->GetCurrentBackBufferRTV();
			auto destination_tex = dest->GetGPUObject().Cast<Diligent::ITexture>(Diligent::IID_Texture);

			if(!resolve)
			{
				context_->SetRenderTargets(0, nullptr, nullptr, RESOURCE_STATE_TRANSITION_MODE_NONE);
				Diligent::CopyTextureAttribs copy_attribs = {};
				copy_attribs.pSrcTexture = source->GetTexture();
				copy_attribs.pSrcBox = &src_box;
				copy_attribs.pDstTexture = destination_tex;
				copy_attribs.DstTextureTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
				copy_attribs.SrcTextureTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
				context_->CopyTexture(copy_attribs);
				context_->SetRenderTargets(num_rts_,bind_rts_.data(), bind_depth_stencil_, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			}
			else
			{
				Diligent::ResolveTextureSubresourceAttribs resolve_attribs = {};
				// if it is resolving to fullscreen, just resolve directly to the destination texture
				// otherwise we must need an auxiliary texture to resolve to
				if (!src_box.MinX && !src_box.MinY && src_box.MaxX == rt_size.x_ && src_box.MaxY == rt_size.y_)
				{
					context_->ResolveTextureSubresource(source->GetTexture(), destination_tex, resolve_attribs);
				}
				else
				{
					throw std::exception("Not implemented. You must implement MSAA first");
				}
			}

			return true;
		}
		bool ResolveTexture(TextureCube* dest) override
		{
			throw std::exception("Not implemented");
		}

		void BeginDebug(const char* mark_name, const Color& color) override
		{
#if ATOMIC_DEBUG
			graphics_->GetImpl()->GetDeviceContext()->BeginDebugGroup(mark_name, color.Data());
#endif
		}
		void EndDebug() override
		{
#if ATOMIC_DEBUG
			graphics_->GetImpl()->GetDeviceContext()->EndDebugGroup();
#endif
		}

		u32 GetPrimitiveCount() override { return primitive_count_; }
		u32 GetNumBatches() override { return num_batches_; }

		VertexBuffer* GetVertexBuffer(u8 index) override
		{
			return vertex_buffers_[index].get();
		}
		IndexBuffer* GetIndexBuffer() override
		{
			return index_buffer_.get();
		}
		ShaderVariation* GetShader(ShaderType type) override
		{
			static ShaderVariation* s_shaders[MAX_SHADER_TYPES] = {
				pipeline_info_->vs_shader,
				pipeline_info_->ps_shader,
			};
			if(type >= MAX_SHADER_TYPES)
				return nullptr;

			return s_shaders[type];
		}
		Texture* GetTexture(TextureUnit unit) override
		{
			if(unit >= MAX_TEXTURE_UNITS)
				return nullptr;
			return textures_[unit].owner.get();
		}
		RenderSurface* GetRenderTarget(u8 index) override
		{
			if(index >= MAX_RENDERTARGETS)
				return nullptr;
			return render_targets_[index].get();
		}
		RenderSurface* GetDepthStencil() override
		{
			return depth_stencil_.get();
		}
		PrimitiveType GetPrimitiveType() override
		{
			return pipeline_info_->primitive_type;
		}
		IntRect GetViewport() override
		{
			return viewport_;
		}
		BlendMode GetBlendMode() override
		{
			return pipeline_info_->blend_mode;
		}
		bool GetAlphaToCoverage() override
		{
			return pipeline_info_->alpha_to_coverage_enabled;
		}
		bool GetColorWrite() override
		{
			return pipeline_info_->color_write_enabled;
		}
		CullMode GetCullMode() override
		{
			return pipeline_info_->cull_mode;
		}
		float GetDepthBias() override
		{
			return pipeline_info_->constant_depth_bias;
		}
		float GetSlopeScaledDepthBias() override
		{
			return pipeline_info_->slope_scaled_depth_bias;
		}
		CompareMode GetDepthTest() override
		{
			return pipeline_info_->depth_cmp_function;
		}
		bool GetDepthWrite() override
		{
			return pipeline_info_->depth_write_enabled;
		}
		FillMode GetFillMode() override
		{
			return pipeline_info_->fill_mode;
		}
		bool GetLineAntiAlias() override
		{
			return pipeline_info_->stencil_test_enabled;
		}
		bool GetScissorTest() override
		{
			return pipeline_info_->scissor_test_enabled;
		}
		IntRect GetScissorRect() override
		{
			return scissor_;
		}
		bool GetStencilTest() override
		{
			return pipeline_info_->stencil_test_enabled;
		}
		CompareMode GetStencilTestMode() override
		{
			return pipeline_info_->stencil_cmp_function;
		}
		StencilOp GetStencilPass() override
		{
			return pipeline_info_->stencil_op_on_passed;
		}
		StencilOp GetStencilFail() override
		{
			return pipeline_info_->stencil_op_on_stencil_failed;
		}
		StencilOp GetStencilZFail() override
		{
			return pipeline_info_->stencil_op_depth_failed;
		}
		u32 GetStencilRef() override
		{
			return stencil_ref_;
		}
		u32 GetStencilCompareMask() override
		{
			return pipeline_info_->stencil_cmp_mask;
		}
		u32 GetStencilWriteMask() override
		{
			return pipeline_info_->stencil_write_mask;
		}
		bool GetClipPlane() override
		{
			return enable_clip_planes_;
		}
		ShaderProgram* GetShaderProgram() override
		{
			return shader_program_;
		}
		IntVector2 GetRenderTargetDimensions() override
		{
			int width, height;
			if(render_targets_[0])
			{
				width = render_targets_[0]->GetWidth();
				height = render_targets_[0]->GetHeight();
			}
			else if(depth_stencil_)
			{
				width = depth_stencil_->GetWidth();
				height = depth_stencil_->GetHeight();
			}
			else
			{
				width = graphics_->GetWidth();
				height = graphics_->GetHeight();
			}

			return IntVector2(width, height);
		}
	private:
		bool PrepareRenderTargets()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareRenderTargets);
			if ((dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::render_targets)) == 0)
				return false;
			// Remove dirty state
			dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::render_targets);

			num_rts_ = 0;
			for(u8 i =0; i < MAX_RENDERTARGETS; ++i)
			{
				if (!render_targets_[i])
					continue;

				const auto format = render_targets_[i]->GetParentTexture()->GetFormat();
				if(pipeline_info_->output.render_target_formats[num_rts_] != format)
					dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
				pipeline_info_->output.render_target_formats[num_rts_] = format;

				bind_rts_[num_rts_++] = render_targets_[i]->GetRenderTargetView();
			}

			const auto wnd_size = graphics_->GetSize();
			const auto depth_stencil = depth_stencil_;
			const auto depth_stencil_size = IntVector2(
				depth_stencil ? depth_stencil->GetWidth() : 0, 
				depth_stencil ? depth_stencil->GetHeight() : 0
			);

			if(pipeline_info_->output.num_rts != num_rts_)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);

			pipeline_info_->output.num_rts = num_rts_;
			if (num_rts_ > 0) 
				return true;

			if (depth_stencil && depth_stencil_size != wnd_size)
				return true;

			num_rts_ = 1;
			bind_rts_[0] = graphics_->GetImpl()->GetSwapChain()->GetCurrentBackBufferRTV();

			if(num_rts_ != pipeline_info_->output.num_rts)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->output.num_rts = num_rts_;

			if (pipeline_info_->output.render_target_formats[0] == bind_rts_[0]->GetTexture()->GetDesc().Format)
				return true;

			pipeline_info_->output.render_target_formats[0] = bind_rts_[0]->GetTexture()->GetDesc().Format;
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);

			return true;
		}
		bool PrepareDepthStencil()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareDepthStencil);
			if (dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::render_targets) == 0)
				return false;

			auto depth_stencil = depth_stencil_ && depth_stencil_->GetUsage() == TEXTURE_DEPTHSTENCIL ?
				depth_stencil_->GetRenderTargetView() :
				graphics_->GetImpl()->GetSwapChain()->GetDepthBufferDSV();
			const auto wnd_size = graphics_->GetRenderTargetDimensions();

			if(!pipeline_info_->depth_write_enabled && depth_stencil_ && depth_stencil_->GetReadOnlyView())
				depth_stencil = depth_stencil_->GetReadOnlyView();
			else if(render_targets_[0] && !depth_stencil_)
			{
				const auto rt_size = IntVector2(
					render_targets_[0]->GetWidth(),
					render_targets_[0]->GetHeight()
				);
				if (rt_size.x_ < wnd_size.x_ || rt_size.y_ < wnd_size.y_)
					depth_stencil = nullptr;
			}

			bind_depth_stencil_ = depth_stencil;

			const auto format = depth_stencil->GetDesc().Format;
			if(format != pipeline_info_->output.depth_stencil_format)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->output.depth_stencil_format = format;
			return true;
		}
		void PreparePipelineState()
		{
			ATOMIC_PROFILE(IDrawCommand::PreparePipelineState);
			if ((dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::pipeline)) == 0)
				return;

			auto hash = pipeline_info_->ToHash();
			if(hash == curr_pipeline_hash_)
				return;
			pipeline_state_ = pipeline_state_builder_acquire(graphics_->GetImpl(), *pipeline_info_, hash);
			curr_pipeline_hash_ = hash;
			dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::pipeline);
			// If pipeline state is changed, we need to update shader resource binding.
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::srb)
				| static_cast<u32>(RenderCommandDirtyState::commit_pipeline);
		}
		void PrepareSRB()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareSRB);
			if(!shader_resource_binding_)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::srb);

			if ((dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::srb)) == 0)
				return;

			ShaderResourceBindingCreateDesc create_desc;
			create_desc.pipeline_hash = curr_pipeline_hash_;
			create_desc.resources = &textures_;
			create_desc.driver = graphics_->GetImpl();
			shader_resource_binding_ = pipeline_state_builder_get_or_create_srb(create_desc);
			dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::srb);
			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::commit_srb);
		}
		void PrepareVertexBuffers()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareVertexBuffers);

			if (curr_vbuffer_checksum_ != last_vbuffer_checksum_)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::vertex_buffer);

			if((dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::vertex_buffer)) == 0)
				return;

			dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::vertex_buffer);

			context_->SetVertexBuffers(
				0,
				num_vertex_buffers_,
				bind_vertex_buffers_.data(),
				vertex_offsets_.data(),
				RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
				SET_VERTEX_BUFFERS_FLAG_NONE);

			if (!curr_vertx_decl_checksum_)
				return;

			if(pipeline_info_->vs_shader)
			{
				CombineHash(curr_vertx_decl_checksum_, pipeline_info_->vs_shader->ToHash());
				CombineHash(curr_vertx_decl_checksum_, pipeline_info_->vs_shader->GetElementHash());
			}

			if(curr_vertx_decl_checksum_ != last_vertx_decl_checksum_)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::vertex_decl);
			last_vertx_decl_checksum_ = curr_vertx_decl_checksum_;
		}
		void PrepareIndexBuffer()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareIndexBuffer);
			if (!index_buffer_)
				return;

			const auto buffer = index_buffer_->GetGPUObject().Cast<Diligent::IBuffer>(Diligent::IID_Buffer);
			if(buffer != s_index_buffer)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::index_buffer);

			if ((dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::index_buffer)) == 0)
				return;

			dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::index_buffer);

			context_->SetIndexBuffer(buffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			s_index_buffer = buffer;
		}
		void PrepareVertexDeclarations()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareVertexDeclarations);
			// If some reason vertex declaration is 0, then we must recompute
			if(curr_vertx_decl_checksum_ == 0)
			{
				for (u32 i = 0; i < num_vertex_buffers_; ++i)
				{
					if(vertex_buffers_[i])
						CombineHash(curr_vertx_decl_checksum_, vertex_buffers_[i]->GetBufferHash(0));
				}

				if (curr_vertx_decl_checksum_ != last_vertx_decl_checksum_)
					dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::vertex_decl);
				last_vertx_decl_checksum_ = curr_vertx_decl_checksum_;
			}

			if((dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::vertex_decl)) == 0)
				return;
			if(pipeline_info_->vs_shader == nullptr)
				return;

			dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::vertex_decl);

			const auto vertex_decl = graphics_state_get_vertex_declaration(curr_vertx_decl_checksum_);
			if(vertex_decl)
			{
				if(vertex_decl != vertex_declaration_)
				{
					dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
					pipeline_info_->input_layout = vertex_decl->GetInputLayoutDesc();
				}
				vertex_declaration_ = vertex_decl;
				return;
			}

			VertexDeclarationCreationDesc creation_desc;
			creation_desc.graphics = graphics_;
			creation_desc.hash = curr_vertx_decl_checksum_;
			creation_desc.vertex_buffers = &vertex_buffers_;
			creation_desc.vertex_shader = pipeline_info_->vs_shader;
			vertex_declaration_ = new VertexDeclaration(creation_desc);

			if(!vertex_declaration_->GetNumInputs())
			{
				vertex_declaration_ = nullptr;
				return;
			}

			graphics_state_set_vertex_declaration(curr_vertx_decl_checksum_, vertex_declaration_);

			dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);
			pipeline_info_->input_layout = vertex_declaration_->GetInputLayoutDesc();
		}
		void PrepareTextures()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareTextures);

			if (!shader_program_)
				return;

			// If current assigned textures is different from last assigned textures, we need to update textures.
			if(curr_assigned_immutable_sa_flags_ != curr_assigned_texture_flags_)
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::textures);

			if((dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::textures)) == 0)
				return;
			dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::textures);

			u32 next_sampler_idx = 0;
			for(auto& desc : textures_)
			{
				if(!desc.owner)
					continue;

				const auto sampler = shader_program_->GetSampler(desc.unit);
				if (!sampler)
					continue;
				if(desc.name_hash != sampler->hash)
					dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::srb);

				desc.name = sampler->name.CString();
				desc.name_hash = sampler->hash;
				// get some hashes
				const u32 sampler_hash = pipeline_info_->immutable_samplers[next_sampler_idx].sampler.ToHash();
				const auto name_hash = pipeline_info_->immutable_samplers[next_sampler_idx].name_hash;
				// fill immutable sampler sampler
				desc.owner->GetSamplerDesc(pipeline_info_->immutable_samplers[next_sampler_idx].sampler);
				// if sampler or immutable sampler name has been changed. we must rebuild pipeline.
				if(sampler_hash != pipeline_info_->immutable_samplers[next_sampler_idx].sampler.ToHash() 
					|| sampler->hash != name_hash)
					dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::pipeline);

				// put the name and hash name inside immutable sampler
				pipeline_info_->immutable_samplers[next_sampler_idx].name_hash = sampler->hash;
				pipeline_info_->immutable_samplers[next_sampler_idx].name = desc.name;

				//curr_assigned_immutable_sa_flags_ |= 1 << desc.unit;
				++next_sampler_idx;
			}

			pipeline_info_->num_samplers = static_cast<u8>(next_sampler_idx);
		}
		void PrepareParametersToUpload()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareParametersToUpload);
			if (!shader_program_)
				return;

			{
				ATOMIC_PROFILE(IDrawCommand::WritingParameters);
				const auto params_count = Min(next_param_2_update_idx_, MAX_SHADER_PARAMETER_UPDATES);
				next_param_2_update_idx_ = 0;
				for(u32 i = 0; i < params_count; ++i)
				{
					const auto param_data = params_2_update_[i];
					ShaderParameter* parameter;
					// if parameter is not found we will add to
					// array again by just swapping elements.
					if(!shader_parameters_cache_get(param_data->hash, &parameter))
					{
						ea::swap(params_2_update_[i], params_2_update_[next_param_2_update_idx_]);
						++next_param_2_update_idx_;
						continue;
					}

					const auto buffer = static_cast<ConstantBuffer*>(parameter->bufferPtr_);
					if (!buffer)
						continue;
					render_command_write_param(buffer, parameter->offset_, &param_data->value);
				}
			}

			graphics_->GetImpl()->UploadBufferChanges();
		}

		void PrepareClear()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareClear);
			auto changed = PrepareDepthStencil();
			changed = PrepareRenderTargets() || changed;
			if (changed)
				BoundRenderTargets();
		}
		void PrepareDraw()
		{
			ATOMIC_PROFILE(IDrawCommand::PrepareDraw);
			auto changed_rts = PrepareDepthStencil();
			changed_rts = PrepareRenderTargets() || changed_rts;

			PrepareVertexBuffers();
			PrepareIndexBuffer();
			PrepareVertexDeclarations();
			PrepareTextures();
			PreparePipelineState();
			PrepareSRB();
			PrepareParametersToUpload();

			if(changed_rts)
				BoundRenderTargets();

			const auto rt_size = graphics_->GetRenderTargetDimensions();
			if(dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::viewport))
			{
				dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::viewport);

				const auto rect = viewport_;
                Diligent::Viewport viewport;
                viewport.TopLeftX = static_cast<float>(rect.left_);
                viewport.TopLeftY = static_cast<float>(rect.top_);
                viewport.Width = static_cast<float>(rect.right_ - rect.left_);
                viewport.Height = static_cast<float>(rect.bottom_ - rect.top_);
                viewport.MinDepth = 0.0f;
                viewport.MaxDepth = 1.0f;
				context_->SetViewports(1, &viewport, rt_size.x_, rt_size.y_);
			}

			if(dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::scissor) && pipeline_info_->scissor_test_enabled)
			{
				dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::scissor);

				const auto rect = scissor_;
				const Diligent::Rect scissor_rect = {
									rect.left_,
									rect.top_,
									rect.right_,
									rect.bottom_
								};
				context_->SetScissorRects(1, &scissor_rect, rt_size.x_, rt_size.y_);
			}

			context_->SetStencilRef(stencil_ref_);

			static constexpr float s_blend_factors[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			context_->SetBlendFactors(s_blend_factors);

			if(pipeline_state_ && dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::commit_pipeline))
			{
				dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::commit_pipeline);
				context_->SetPipelineState(pipeline_state_);
			}
			if(shader_resource_binding_ && dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::commit_srb))
			{
				dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::commit_srb);
				context_->CommitShaderResources(shader_resource_binding_, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			}
		}
		void BoundRenderTargets() const
		{
			context_->SetRenderTargets(
				num_rts_, 
				const_cast<ITextureView**>(bind_rts_.data()), 
				bind_depth_stencil_, 
				Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		}
		void ClearByHardware(const DrawCommandClearDesc& desc) const
		{
			ATOMIC_PROFILE(IDrawCommand::ClearByHardware);
				
			auto clear_stencil_flags = Diligent::CLEAR_DEPTH_FLAG_NONE;
			if(desc.flags & CLEAR_COLOR && bind_rts_[0])
				context_->ClearRenderTarget(bind_rts_[0], desc.color.Data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			if((desc.flags & (CLEAR_DEPTH | CLEAR_STENCIL)) !=0 && bind_depth_stencil_)
			{
				if(desc.flags & CLEAR_DEPTH)
					clear_stencil_flags |= Diligent::CLEAR_DEPTH_FLAG;
				if(desc.flags & CLEAR_STENCIL)
					clear_stencil_flags |= Diligent::CLEAR_STENCIL_FLAG;

				context_->ClearDepthStencil(bind_depth_stencil_, 
					clear_stencil_flags, 
					desc.depth, 
					static_cast<u8>(desc.stencil), 
					Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			}
		}
		void ClearBySoftware(const DrawCommandClearDesc& desc)
		{
			ATOMIC_PROFILE(IDrawCommand::ClearBySoftware);
			const auto renderer = graphics_->GetContext()->GetSubsystem<Renderer>();
			if (!renderer)
			{
				ATOMIC_LOGWARNING("Can´t clear without Renderer. Skipping!");
				return;
			}

			auto model_transform = Matrix3x4::IDENTITY;
			model_transform.m23_ = Clamp(desc.depth, 0.0f, 1.0f);
			auto proj = Matrix4::IDENTITY;
			auto clear_color = desc.color;
			const auto vs_shader = graphics_->GetShader(VS, "ClearFramebuffer");
			const auto ps_shader = graphics_->GetShader(PS, "ClearFramebuffer");
			if (!vs_shader->GetGPUObject())
				vs_shader->Create();
			if(!ps_shader->GetGPUObject())
				ps_shader->Create();

			const auto program = GetOrCreateShaderProgram({ vs_shader, ps_shader });

			PipelineStateInfo pipeline_info = {};
			pipeline_info.debug_name = "Clear";
			pipeline_info.vs_shader = vs_shader;
			pipeline_info.ps_shader = ps_shader;
			if (render_targets_[0])
				pipeline_info.output.render_target_formats[0] = render_targets_[0]->GetParentTexture()->GetFormat();
			else
				pipeline_info.output.render_target_formats[0] = graphics_->GetImpl()->GetSwapChain()->GetDesc().ColorBufferFormat;
			if (depth_stencil_)
				pipeline_info.output.depth_stencil_format = depth_stencil_->GetParentTexture()->GetFormat();
			else
				pipeline_info.output.depth_stencil_format = graphics_->GetImpl()->GetSwapChain()->GetDesc().DepthBufferFormat;
			pipeline_info.output.num_rts = 1;
			pipeline_info.blend_mode = BLEND_REPLACE;
			pipeline_info.color_write_enabled = desc.flags & CLEAR_COLOR;
			pipeline_info.alpha_to_coverage_enabled = false;
			pipeline_info.cull_mode = CULL_NONE;
			pipeline_info.depth_cmp_function = CMP_ALWAYS;
			pipeline_info.depth_write_enabled = desc.flags & CLEAR_DEPTH;
			pipeline_info.fill_mode = FILL_SOLID;
			pipeline_info.scissor_test_enabled = false;
			pipeline_info.stencil_test_enabled = desc.flags & CLEAR_STENCIL;
			pipeline_info.stencil_cmp_function = CMP_ALWAYS;
			pipeline_info.stencil_op_on_passed = OP_REF;
			pipeline_info.stencil_op_on_stencil_failed = OP_KEEP;
			pipeline_info.stencil_op_depth_failed = OP_KEEP;
			pipeline_info.primitive_type = TRIANGLE_STRIP;
			pipeline_info.input_layout.num_elements = 1;
			pipeline_info.input_layout.elements[0] = InputLayoutElementDesc{
				0, 0, sizeof(Vector3), 0, 0, TYPE_VECTOR3
			};

			u32 pipeline_hash;
			RefCntAutoPtr<IPipelineState> pipeline_state = pipeline_state_builder_acquire(graphics_->GetImpl(), pipeline_info, pipeline_hash);

			ShaderResourceTextures textures_dummy = {};
			RefCntAutoPtr<IShaderResourceBinding> srb = pipeline_state_builder_get_or_create_srb({
				graphics_->GetImpl(),
				pipeline_hash,
				&textures_dummy
				});

			{
				Matrix4 cpy_model_transform = model_transform.ToMatrix4();
				WriteShaderParameter(program, VSP_MODEL, &cpy_model_transform, sizeof(Matrix4));
				WriteShaderParameter(program, VSP_VIEWPROJ, &proj, sizeof(Matrix4));
				WriteShaderParameter(program, PSP_MATDIFFCOLOR, &clear_color, sizeof(Matrix4));

				graphics_->GetImpl()->GetConstantBuffer(VS, SP_CAMERA)->Apply();
				graphics_->GetImpl()->GetConstantBuffer(VS, SP_OBJECT)->Apply();
				graphics_->GetImpl()->GetConstantBuffer(PS, SP_MATERIAL)->Apply();
			}

			if(dirty_flags_ & static_cast<u32>(RenderCommandDirtyState::viewport))
			{
				dirty_flags_ ^= static_cast<u32>(RenderCommandDirtyState::viewport);
				const auto rt_size = GetRenderTargetDimensions();
				const auto rect = viewport_;
                Diligent::Viewport viewport;
                viewport.TopLeftX = static_cast<float>(rect.left_);
                viewport.TopLeftY = static_cast<float>(rect.top_);
                viewport.Width = static_cast<float>(rect.right_ - rect.left_);
                viewport.Height = static_cast<float>(rect.bottom_ - rect.top_);
                viewport.MinDepth = 0.0f;
                viewport.MaxDepth = 1.0f;
				context_->SetViewports(1, &viewport, rt_size.x_, rt_size.y_);
			}

			DrawAttribs draw_attribs = {};
			draw_attribs.Flags = DRAW_FLAG_NONE;
			draw_attribs.NumInstances = 1;
			draw_attribs.StartVertexLocation = 0;
			draw_attribs.NumVertices = 4;

			context_->SetStencilRef(desc.stencil);
			context_->SetPipelineState(pipeline_state);
			context_->CommitShaderResources(srb, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			context_->Draw(draw_attribs);
		}
		void SetVertexBuffers(VertexBuffer** buffers, u32 count, u32 instance_offset)
		{
			ATOMIC_PROFILE(IDrawCommand::SetVertexBuffers);
			if (count > MAX_VERTEX_STREAMS)
				ATOMIC_LOGWARNING("Too many vertex buffers");

			curr_vbuffer_checksum_		= 
			curr_vertx_decl_checksum_	= 0;

			num_vertex_buffers_ = count = Min(count, MAX_VERTEX_STREAMS);
			for (u32 i = 0; i < count; ++i)
			{
				const auto buffer = buffers[i];
				const auto& elements = buffer->GetElements();
				// Check if buffer has per-instance data
				const auto has_instance_data = elements.Size() && elements[0].perInstance_;
				const auto offset = has_instance_data ? instance_offset * buffer->GetVertexSize() : 0;
				const auto buffer_obj = buffer->GetGPUObject().Cast<Diligent::IBuffer>(Diligent::IID_Buffer);

				buffer->AddRef();
				vertex_buffers_[i].reset(buffer, ea::EngineRefCounterDeleter<VertexBuffer>());
				vertex_offsets_[i] = offset;
				bind_vertex_buffers_[i] = buffer_obj;
				// Build vertex buffer checksum
				curr_vbuffer_checksum_ = 16777619;
				curr_vbuffer_checksum_ ^= reinterpret_cast<u32>(buffer_obj.ConstPtr()) * 16777619;
				curr_vbuffer_checksum_ ^= offset * 16777619;
				//CombineHash(curr_vbuffer_checksum_, MakeHash(buffer_obj.ConstPtr()));
				//CombineHash(curr_vbuffer_checksum_, offset);
				// Build base Vertex Declaration Hash
				curr_vertx_decl_checksum_ ^= buffer->GetBufferHash(i);
				//CombineHash(curr_vertx_decl_checksum_, buffer->GetBufferHash(i));
			}

			if(curr_vbuffer_checksum_ != last_vbuffer_checksum_)
			{
				last_vbuffer_checksum_ = curr_vbuffer_checksum_;
				dirty_flags_ |= static_cast<u32>(RenderCommandDirtyState::vertex_buffer);
			}
		}
		SharedPtr<ShaderProgram> GetOrCreateShaderProgram(const ShaderProgramQuery& query) const
		{
			auto result = graphics_state_get_shader_program(query);
			if (result)
				return result;

			ShaderProgramCreationDesc creation_desc;
			creation_desc.graphics = graphics_;
			creation_desc.vertex_shader = query.vertex_shader;
			creation_desc.pixel_shader = query.pixel_shader;
			result = new ShaderProgram(creation_desc);
			graphics_state_set_shader_program(result);

			return result;
		}

		static void WriteShaderParameter(ShaderProgram* program, const StringHash& param, void* data, u32 length)
		{
			ShaderParameter* shader_param;
			if (!shader_parameters_cache_get(param.Value(), &shader_param))
				return;
			if (!shader_param->bufferPtr_)
				return;
			static_cast<ConstantBuffer*>(shader_param->bufferPtr_)->SetParameter(shader_param->offset_, length, data);
		}
		static Diligent::VALUE_TYPE GetIndexSize(u32 size)
		{
			return size == sizeof(u16) ? Diligent::VT_UINT16 : Diligent::VT_UINT32;
		}

		Graphics* graphics_;
		Diligent::IDeviceContext* context_;
		PipelineStateInfo* pipeline_info_;
		Diligent::RefCntAutoPtr<Diligent::IPipelineState> pipeline_state_;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> shader_resource_binding_;

		ea::shared_ptr<RenderSurface> depth_stencil_;
		ITextureView* bind_depth_stencil_;
		ea::array<ShaderParameterUpdateData*, MAX_SHADER_PARAMETER_UPDATES> params_2_update_;
		u32 next_param_2_update_idx_;

		// arrays
		ea::array<ea::shared_ptr<RenderSurface>, MAX_RENDERTARGETS> render_targets_;
		ea::array<ITextureView*, MAX_RENDERTARGETS> bind_rts_;
		ea::array<ShaderResourceTextureDesc, MAX_TEXTURE_UNITS> textures_;
		ea::array<ea::shared_ptr<VertexBuffer>, MAX_VERTEX_STREAMS> vertex_buffers_;
		ea::array<Diligent::IBuffer*, MAX_VERTEX_STREAMS> bind_vertex_buffers_;
		ea::array<u64, MAX_VERTEX_STREAMS> vertex_offsets_;
		ea::array<u32, MAX_SHADER_PARAMETER_GROUPS> shader_param_sources_;

		ea::shared_ptr<IndexBuffer> index_buffer_;

		SharedPtr<VertexDeclaration> vertex_declaration_;
		SharedPtr<ShaderProgram> shader_program_;

		bool enable_clip_planes_{};
		Vector4 clip_plane_{};

		IntRect scissor_{};
		IntRect viewport_{};

		ValueType index_type_{};

		u8 stencil_ref_{};

		u32 dirty_flags_{};

		u32 curr_assigned_texture_flags_{};
		u32 curr_assigned_immutable_sa_flags_{};

		u32 num_rts_{};
		u32 num_vertex_buffers_{};

		u32 curr_vertx_decl_checksum_{};
		u32 last_vertx_decl_checksum_{};
		u32 curr_vbuffer_checksum_{};
		u32 last_vbuffer_checksum_{};
		u32 curr_pipeline_hash_{};

		u8 textures_in_use_{};

		u32 primitive_count_;
		u32 num_batches_;
	};

	Atomic::IDrawCommand* graphics_create_command(Atomic::Graphics* graphics)
	{
		Atomic::IDrawCommand* result = new DrawCommandImpl(graphics, graphics->GetImpl()->GetDeviceContext());
		return result;
	}
}
