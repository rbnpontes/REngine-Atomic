#pragma once
#include "./RHITypes.h"
#include "../Container/RefCounted.h"
#include "../Graphics/Graphics.h"

namespace REngine
{
	struct VertexDeclarationCreationDesc
	{
		Atomic::Graphics* graphics{nullptr};
		Atomic::ShaderVariation* vertex_shader{nullptr};
		Atomic::VertexBuffer** vertex_buffers{nullptr};
	};

	class RENGINE_API VertexDeclaration : public Atomic::RefCounted
	{
		ATOMIC_REFCOUNTED(VertexDeclaration);
	public:
		VertexDeclaration(const VertexDeclarationCreationDesc& creation_desc);
		InputLayoutDesc GetInputLayoutDesc() const { return input_layout_desc_; }
	private:
		InputLayoutDesc input_layout_desc_{};
	};
}
