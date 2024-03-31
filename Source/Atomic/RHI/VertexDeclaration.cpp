#include "./VertexDeclaration.h"
#include "../Graphics/VertexBuffer.h"

namespace REngine
{
	VertexDeclaration::VertexDeclaration(const VertexDeclarationCreationDesc& creation_desc)
	{
		const auto& shader_input_elements = creation_desc.vertex_shader->GetInputElements();
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
			const auto vertex_buffer = creation_desc.vertex_buffers[buffer_idx];
			if (!vertex_buffer)
				continue;

			const auto vertex_elements = vertex_buffer->GetElements();
			for(const auto& vertex_element : vertex_elements)
			{
				uint32_t hash = vertex_element.index_;
				Atomic::CombineHash(hash, vertex_element.semantic_);

				auto input_element = shader_input_elements_map.Find(hash);

				// If the shader doesn't have the input element, skip it
				if(input_element == shader_input_elements_map.End())
					continue;

				auto& element = input_layout_desc_.elements[input_layout_desc_.num_elements];
				element.buffer_stride = vertex_buffer->GetVertexSize();
				element.buffer_index = buffer_idx;
				element.input_index = input_element->second_.index;
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
			input_element.buffer_stride = 0;
			input_element.buffer_index = 0;
			input_element.input_index =	element.second_.index;
			input_element.element_offset = 0;
			input_element.element_type = element.second_.element_type;
			input_element.instance_step_rate = 0;
			++input_layout_desc_.num_elements;
		}

		//// Loop through the shader input elements and find the corresponding vertex element
		//for(uint8_t attr_idx = 0; attr_idx < static_cast<uint8_t>(shader_input_elements.Size()); ++attr_idx)
		//{
		//	const auto& shader_element = shader_input_elements[attr_idx];
		//	auto& input_element = input_layout_desc_.elements[input_layout_desc_.num_elements];
		//	bool found = false;
		//	for(uint8_t buffer_idx = 0; buffer_idx < Atomic::MAX_VERTEX_STREAMS; ++buffer_idx)
		//	{
		//		const auto vertex_buffer = creation_desc.vertex_buffers[buffer_idx];
		//		if(!vertex_buffer)
		//			continue;
		//		const auto vertex_element = vertex_buffer->GetElement(shader_element.semantic, shader_element.semantic_index);
		//		if(!vertex_element)
		//			continue;

		//		input_element.buffer_stride = vertex_buffer->GetVertexSize();
		//		input_element.buffer_index = buffer_idx;
		//		input_element.input_index = shader_element.index;
		//		input_element.element_offset = vertex_element->offset_;
		//		input_element.element_type = vertex_element->type_;
		//		input_element.instance_step_rate = vertex_element->perInstance_ ? 1 : 0;
		//		found = true;
		//		++input_layout_desc_.num_elements;
		//	}

		//	if(!found)
		//	{
		//		// if corresponding vertex element is not found, use default values
		//		input_element.buffer_stride = 0;
		//		input_element.buffer_index = 0;
		//		input_element.input_index = shader_element.index;
		//		input_element.element_offset = 0;
		//		input_element.element_type = shader_element.element_type;
		//		input_element.instance_step_rate = 0;
		//		++input_layout_desc_.num_elements;
		//	}
		//}

		//// Make sure we have the same number of input elements as the shader
		//assert(input_layout_desc_.num_elements == shader_input_elements.Size());
	}
}
