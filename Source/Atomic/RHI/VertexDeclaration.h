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
		ea::array<Atomic::SharedPtr<Atomic::VertexBuffer>, Atomic::MAX_VERTEX_STREAMS>* vertex_buffers{nullptr};
		u32 hash{ 0 };
	};

	class RENGINE_API VertexDeclaration : public Atomic::RefCounted
	{
		ATOMIC_REFCOUNTED(VertexDeclaration);
	public:
		VertexDeclaration(const VertexDeclarationCreationDesc& creation_desc);
		InputLayoutDesc GetInputLayoutDesc() const { return input_layout_desc_; }
		u32 ToHash() const { return hash_; }
	private:
		InputLayoutDesc input_layout_desc_{};
		u32 hash_{};
	};
}
