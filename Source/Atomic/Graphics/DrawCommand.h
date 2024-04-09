#pragma once
#include "../Core/Object.h"
#include "./GraphicsDefs.h"
#include "./TextureCube.h"
#include "./VertexBuffer.h"
#include "./IndexBuffer.h"
#include "./ShaderVariation.h"

#include <DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h>

namespace Atomic
{
	typedef Diligent::VALUE_TYPE ValueType;
	struct DrawCommandClearDesc
	{
		u32 flags {0};
		Color color {Color::BLACK};
		float depth {1};
		u32 stencil {0};
	};

	struct DrawCommandDrawDesc
	{
		u32 min_vertex{ 0 };
		u32 base_vertex_index{ 0 };
		u32 vertex_start{ 0 };
		u32 vertex_count{ 0 };
		u32 index_start{ 0 };
		u32 index_count{ 0 };
		ValueType index_type{ ValueType::VT_UNDEFINED };
	};
	struct DrawCommandInstancedDrawDesc
	{
		u32 min_vertex{ 0 };
		u32 base_vertex_index{ 0 };
		u32 vertex_count{ 0 };
		u32 index_start{ 0 };
		u32 index_count{ 0 };
		ValueType index_type{ ValueType::VT_UNDEFINED };
		u32 instance_count{ 0 };
	};
	struct DrawCommandShadersDesc
	{
		ShaderVariation* vs;
		ShaderVariation* ps;
		ShaderVariation* gs;
		ShaderVariation* hs;
		ShaderVariation* ds;
	};
	struct DrawCommandStencilTestDesc
	{
		bool enable{ false };
		CompareMode mode { CMP_ALWAYS };
		StencilOp pass { OP_KEEP };
		StencilOp fail { OP_KEEP };
		StencilOp depth_fail { OP_KEEP };
		u32 stencil_ref{ 0 };
		u32 compare_mask{ M_MAX_UNSIGNED };
		u32 write_mask { M_MAX_UNSIGNED };
	};
	struct DrawCommandClipPlaneDesc
	{
		bool enable{ false };
		Plane clip_plane{ Plane::UP };
		Matrix3x4 view{ Matrix3x4::IDENTITY };
		Matrix4 projection{ Matrix4::IDENTITY };
	};

	class IDrawCommand
	{
		/// Reset draw command to default state
		virtual void Reset() = 0;
		// Execution Commands
		/// Clear the render target.
		virtual void Clear(const DrawCommandClearDesc& desc) = 0;
		/// Draw geometry.
		virtual void Draw(const DrawCommandDrawDesc& desc) = 0;
		/// Draw geometry with instancing.
		virtual void Draw(const DrawCommandInstancedDrawDesc& desc) = 0;
		// State Commands
		/// Bind vertex buffer.
		virtual void SetVertexBuffer(VertexBuffer* buffer, u32 instance_offset = 0) = 0;
		/// Bind vertex buffers.
		virtual void SetVertexBuffers(const PODVector<VertexBuffer*> buffers, u32 instance_offset = 0) = 0;
		/// Bind vertex buffers.
		virtual void SetVertexBuffers(const Vector<SharedPtr<VertexBuffer>>& buffers, u32 instance_offset = 0) = 0;
		/// Bind vertex buffers.
		virtual void SetVertexBuffers(const ea::vector<SharedPtr<VertexBuffer>>& buffers, u32 instance_offset = 0) = 0;
		/// Bind vertex buffers.
		virtual void SetVertexBuffers(const ea::vector<VertexBuffer*>& buffers, u32 instance_offset = 0) = 0;
		/// Bind index buffer.
		virtual void SetIndexBuffer(IndexBuffer* buffer) = 0;
		/// Bind shaders.
		virtual void SetShaders(const DrawCommandShadersDesc& desc) = 0;
		/// Set float array shader parameter.
		virtual void SetShaderParameter(StringHash param, const float* data, u32 count) = 0;
		/// Set float array shader parameter.
		virtual void SetShaderParameter(StringHash param, const FloatVector& value) = 0;
		/// Set float shader parameter.
		virtual void SetShaderParameter(StringHash param, float value) = 0;
		/// Set int shader parameter.
		virtual void SetShaderParameter(StringHash param, int value) = 0;
		/// Set bool shader parameter.
		virtual void SetShaderParameter(StringHash param, bool value) = 0;
		/// Set color shader parameter.
		virtual void SetShaderParameter(StringHash param, const Color& value) = 0;
		/// Set Vector2 shader parameter.
		virtual void SetShaderParameter(StringHash param, const Vector2& value) = 0;
		/// Set Vector3 shader parameter.
		virtual void SetShaderParameter(StringHash param, const Vector3& value) = 0;
		/// Set Vector4 shader parameter.
		virtual void SetShaderParameter(StringHash param, const Vector4& value) = 0;
		/// Set IntVector2 shader parameter.
		virtual void SetShaderParameter(StringHash param, const IntVector2& value) = 0;
		/// Set IntVector3 shader parameter.
		virtual void SetShaderParameter(StringHash param, const IntVector3& value) = 0;
		/// Set Matrix3 shader parameter.
		virtual void SetShaderParameter(StringHash param, const Matrix3& value) = 0;
		/// Set Matrix3x4 shader parameter.
		virtual void SetShaderParameter(StringHash param, const Matrix3x4& value) = 0;
		/// Set Matrix4 shader parameter.
		virtual void SetShaderParameter(StringHash param, const Matrix4& value) = 0;
		/// Set Variant shader parameter.
		virtual void SetShaderParameter(StringHash param, const Variant& value) = 0;
		/// Test if shader parameter exists on bound shaders.
		virtual bool HasShaderParameter(StringHash param) = 0;
		/// Bound texture by unit.
		virtual void SetTexture(TextureUnit unit, Texture* texture) =0;
		/// Bound render target by unit
		virtual void SetTexture(TextureUnit unit, RenderSurface* surface) =0;
		/// Bound texture by name.
		virtual void SetTexture(const String& name, Texture* texture) =0;
		/// Test if current shaders have a texture bound to a unit.
		virtual bool HasTexture(TextureUnit unit) =0;
		/// Bound render target.
		virtual void SetRenderTarget(u8 index, RenderSurface* surface) = 0;
		/// Bound texture as render target. NOTE: texture must be created as render target.
		virtual void SetRenderTarget(u8 index, Texture2D* texture) = 0;
		/// Bound depth stencil
		virtual void SetDepthStencil(RenderSurface* surface) = 0;
		/// Bound texture as depth stencil. NOTE: texture must be created as render target.
		virtual void SetDepthStencil(Texture2D* texture) = 0;
		/// Clear all bound render targets.
		virtual void ResetRenderTargets() = 0;
		/// Unbound render target by index.
		virtual void ResetRenderTarget(u8 index) = 0;
		/// Unbound depth stencil.
		virtual void ResetDepthStencil() = 0;
		/// Set primitive type.
		virtual void SetPrimitiveType(PrimitiveType type) = 0;
		/// Set viewport
		virtual void SetViewport(const IntRect& viewport) = 0;
		/// Set blend mode
		virtual void SetBlendMode(BlendMode mode) = 0;
		/// Enable color write
		virtual void SetColorWrite(bool enable) = 0;
		/// Set cull mode
		virtual void SetCullMode(CullMode mode) = 0;
		/// Set depth bias
		virtual void SetDepthBias(float constant_bias, float slope_scaled_bias) = 0;
		/// Set depth test
		virtual void SetDepthTest(CompareMode mode) = 0;
		/// Set depth write
		virtual void SetDepthWrite(bool enable) = 0;
		/// Set fill mode
		virtual void SetFillMode(FillMode mode) = 0;
		/// Enable line anti alias
		virtual void SetLineAntiAlias(bool enable) = 0;
		/// Enable scissor test
		virtual void SetScissorTest(bool enable, const Rect& rect = Rect::FULL, bool border_inclusive = true) = 0;
		/// Enable scissor test
		virtual void SetScissorTest(bool enable, const IntRect& rect) = 0;
		/// Enable stencil test
		virtual void SetStencilTest(const DrawCommandStencilTestDesc& desc) = 0;
		/// Set clip plane
		virtual void SetClipPlane(const DrawCommandClipPlaneDesc& desc) = 0;

		/// Resolve backbuffer to a texture.
		virtual bool ResolveTexture(Texture2D* desc) = 0;
		/// Resolve backbuffer to a texture by specifying a viewport.
		virtual bool ResolveTexture(Texture2D* dest, const IntRect& viewport) = 0;
		/// Resolve backbuffer to a texture cube.
		virtual bool ResolveTexture(TextureCube* dest) = 0;

		// Getters
		/// Get vertex buffer by index. Index must be between 0 <= MAX_VERTEX_STREAMS.
		virtual VertexBuffer* GetVertexBuffer(u8 index) = 0;
		virtual IndexBuffer* GetIndexBuffer() = 0;
		virtual ShaderVariation* GetShader(ShaderType type) = 0;
		virtual Texture* GetTexture(TextureUnit unit) = 0;
		virtual Texture* GetTexture(const String& name) = 0;
		/// Get render target by index. Index must be between 0 <= MAX_RENDERTARGETS.
		virtual RenderSurface* GetRenderTarget(u8 index) = 0;
		virtual RenderSurface* GetDepthStencil() = 0;
		virtual PrimitiveType GetPrimitiveType() = 0;
		virtual IntRect GetViewport() = 0;
		virtual BlendMode GetBlendMode() = 0;
		virtual bool GetColorWrite() = 0;
		virtual CullMode GetCullMode() = 0;
		virtual float GetDepthBias() = 0;
		virtual float GetSlopeScaledDepthBias() = 0;
		virtual CompareMode GetDepthTest() = 0;
		virtual bool GetDepthWrite() = 0;
		virtual FillMode GetFillMode() = 0;
		virtual bool GetLineAntiAlias() = 0;
		virtual bool GetScissorTest() = 0;
		virtual IntRect GetScissorRect() = 0;
		virtual bool GetStencilTest() = 0;
		virtual CompareMode GetStencilTestMode() = 0;
		virtual StencilOp GetStencilPass() = 0;
		virtual StencilOp GetStencilFail() = 0;
		virtual StencilOp GetStencilZFail() = 0;
		virtual u32 GetStencilRef() = 0;
		virtual u32 GetStencilCompareMask() = 0;
		virtual u32 GetStencilWriteMask() = 0;
		virtual bool GetClipPlane() = 0;
	};

}

namespace REngine
{
	static Atomic::IDrawCommand* graphics_create_command(Atomic::Graphics* graphics);
}