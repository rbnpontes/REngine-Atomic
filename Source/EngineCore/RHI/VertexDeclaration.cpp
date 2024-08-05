#include "./VertexDeclaration.h"
#include "../Graphics/VertexBuffer.h"

namespace REngine
{
	VertexDeclaration::VertexDeclaration(const VertexDeclarationCreationDesc& creation_desc)
	{
		hash_ = creation_desc.hash;
		const auto& shader_input_elements = creation_desc.program->GetInputElements();
		Atomic::HashMap<uint32_t, ShaderCompilerReflectInputElement> shader_input_elements_map;
		input_layout_desc_.num_elements = 0;

		// fill Shader Input Elements map
		for(const auto& element : shader_input_elements)
		{
			uint32_t hash = element.semantic_index;
			Atomic::CombineHash(hash, static_cast<uint32_t>(element.semantic));
			shader_input_elements_map[hash] = element;
		}

		for(uint32_t buffer_idx = 0; buffer_idx < Atomic::MAX_VERTEX_STREAMS; ++buffer_idx)
		{
			const auto vertex_buffer = creation_desc.vertex_buffers->at(buffer_idx);
			if (!vertex_buffer)
				continue;

			const auto vertex_elements = vertex_buffer->GetElements();
			for(const auto& vertex_element : vertex_elements)
			{
				uint32_t hash = vertex_element.index_;
				Atomic::CombineHash(hash, vertex_element.semantic_);

				const auto& input_element_it = shader_input_elements_map.Find(hash);

				// If the shader doesn't have the input element, skip it
				if(input_element_it == shader_input_elements_map.End())
					continue;
				const auto& input_element = input_element_it->second_;
				auto& element = input_layout_desc_.elements[input_layout_desc_.num_elements];
				element.buffer_stride = vertex_buffer->GetVertexSize();
				element.buffer_index = buffer_idx;
				element.input_index = input_element.index;
				element.element_offset = vertex_element.offset_;
				element.element_type = vertex_element.type_;
				element.instance_step_rate = vertex_element.perInstance_ ? 1 : 0;
				++input_layout_desc_.num_elements;
				// remove item from map
				shader_input_elements_map.Erase(hash);
			}
		}

		// Some input elements are not found in the vertex buffer, use default values
		for(const auto& element : shader_input_elements_map)
		{
			auto& input_element = input_layout_desc_.elements[input_layout_desc_.num_elements];
			input_element.buffer_stride = creation_desc.vertex_buffers->at(0)->GetVertexSize();
			input_element.buffer_index = 0;
			input_element.input_index =	element.second_.index;
			input_element.element_offset = 0;
			input_element.element_type = element.second_.element_type;
			input_element.instance_step_rate = 0;
			++input_layout_desc_.num_elements;
		}

		assert(input_layout_desc_.num_elements < Diligent::MAX_LAYOUT_ELEMENTS);
	}
}
