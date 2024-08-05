// require this define:
// https://github.com/microsoft/cppwinrt/issues/479
// otherwise spirv_cross will crash
#define NOMINMAX

#include "./ShaderCompiler.h"
#include "./DiligentUtils.h"

#include "../Engine/Application.h"
#include "../IO/Log.h"
#include "../Core/Variant.h"

#include "spirv_parser.hpp"
#include "spirv_cross.hpp"
#include "spirv_hlsl.hpp"
#include <spirv-tools/libspirv.hpp>
#include <spirv-tools/optimizer.hpp>

#include <shaderc/shaderc.h>

#define SHADER_VERSION 0x00000001

namespace REngine
{
    class HLSLRemapperCompiler : public spirv_cross::CompilerHLSL
    {
    public:
        HLSLRemapperCompiler(uint32_t* spirv, uint64_t length) : spirv_cross::CompilerHLSL(spirv, length)
        {
            RemapInputLayout();
            CollectSamplers();
        }

        std::string compile() override
        {
        	auto hlsl = spirv_cross::CompilerHLSL::compile();
			ReplaceSamplers(hlsl);
			return hlsl;
		}
    private:
        void RemapInputLayout()
        {
            const auto& execution = get_entry_point();
            if (execution.model != spv::ExecutionModelVertex)
                return;
            ir.for_each_typed_id<spirv_cross::SPIRVariable>([&](uint32_t, spirv_cross::SPIRVariable& var)
            {
                auto& type = this->get<spirv_cross::SPIRType>(var.basetype);
                auto& m = ir.meta[var.self].decoration;
                if(!(type.storage == spv::StorageClassInput && !is_builtin_variable(var)))
				    return;

                auto type_name = get_name(var.self);
                if (type_name.empty())
                    type_name = get_name(var.basevariable);
                if(type_name.empty())
					type_name = get_name(var.basetype);

                auto name = String("ATTRIB");
                name.AppendWithFormat("%d", m.location);
                add_vertex_attribute_remap({ m.location, name.CString() });
            });
        }
        void CollectSamplers()
        {
            ir.for_each_typed_id<spirv_cross::SPIRVariable>([&](uint32_t, spirv_cross::SPIRVariable& var)
            {
                auto& type = this->get<spirv_cross::SPIRType>(var.basetype);
                auto& m = ir.meta[var.self].decoration;
                if (type.basetype == spirv_cross::SPIRType::SampledImage && type.image.dim == spv::DimBuffer)
                    return;
                const auto name = String("_") + String(m.alias.c_str()) + "_sampler";
            	samplers_.Push(name);
            });
        }
        void ReplaceSamplers(std::string& hlsl)
        {
            // spirv_cross generates our HLSL code with this pattern: _<alias>_sampler
            // we need to remove the underscore for diligent to find the correct sampler
            const char placeholder = '\1';

            // Replace all underscores with a placeholder
            for(const auto& name : samplers_)
            {
	            for(size_t pos = hlsl.find(name.CString()); pos != std::string::npos; pos = hlsl.find(name.CString(), pos))
	            	hlsl[pos] = placeholder;
            }

            // then remove all placeholders chars
            const auto is_placeholder = [placeholder](char c) { return c == placeholder; };
            hlsl.erase(std::remove_if(hlsl.begin(), hlsl.end(), is_placeholder), hlsl.end());
        }

        Atomic::StringVector samplers_{};
    };

    static shaderc_shader_kind get_shader_kind(Atomic::ShaderType type)
    {
        constexpr shaderc_shader_kind shader_type_tbl[MAX_SHADER_TYPES] = {
            shaderc_vertex_shader,
            shaderc_fragment_shader
        };

        return shader_type_tbl[type];
    }

    static Atomic::VertexElementType get_element_type(const spirv_cross::SPIRType& type)
    {
        Atomic::VertexElementType result = Atomic::MAX_VERTEX_ELEMENT_TYPES;
        switch (type.basetype)
        {
        case spirv_cross::SPIRType::Int:
            result = TYPE_INT;
            break;
        case spirv_cross::SPIRType::Float:
            result = TYPE_FLOAT;
            break;
        default:
            if (type.columns > 1)
            {
                switch (type.vecsize)
                {
                case 2:
                    result = TYPE_VECTOR2;
                    break;
                case 3:
                    result = TYPE_VECTOR3;
                    break;
                case 4:
                    result = TYPE_VECTOR4;
                    break;
                }
            }
            else if (type.basetype == spirv_cross::SPIRType::UInt && type.width == 8 && type.columns == 1)
            {
                result = type.array.empty() ? TYPE_UBYTE4 : TYPE_UBYTE4_NORM;
            }
        }

        return result;
    }

    void shader_compiler_preprocess(const ShaderCompilerDesc& desc, ShaderCompilerPreProcessResult& output)
    {
        auto source_code = desc.source_code;
        if(desc.backend == GraphicsBackend::OpenGLES)
        {
            // Replace header version to 310 es, Otherwise the code above will not work!
            source_code.Replace("#version 300 es", "#version 450");
        }

        const auto compiler = shaderc_compiler_initialize();
        const auto options = shaderc_compile_options_initialize();
        shaderc_compile_options_set_source_language(options, shaderc_source_language_glsl);
        shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level_performance);
        shaderc_compile_options_set_target_env(options, shaderc_target_env_opengl, 0);
        shaderc_compile_options_set_auto_map_locations(options, true);
        shaderc_compile_options_set_auto_bind_uniforms(options, true);

        const auto result = shaderc_compile_into_preprocessed_text(compiler,
            source_code.CString(),
            source_code.Length(),
            get_shader_kind(desc.type),
            desc.name.CString(),
            "main",
            options);

        if(shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success)
        {
            output.error = shaderc_result_get_error_message(result);
            output.has_error = true;
        }
    	else
        {
            Atomic::String source_code = shaderc_result_get_bytes(result);
            if(desc.backend == GraphicsBackend::OpenGLES)
            {
                // Replace header version to 310 es.
                source_code.Replace("#version 450", "#version 300 es");
            }
    		output.source_code = source_code;
            output.has_error = false;
            output.error = shaderc_result_get_error_message(result);
        }


        shaderc_result_release(result);
    	shaderc_compile_options_release(options);
        shaderc_compiler_release(compiler);
    }

    void shader_compiler_compile(const ShaderCompilerDesc& desc, const bool optimize, ShaderCompilerResult& output)
    {
        auto source_code = desc.source_code;
        if(desc.backend == GraphicsBackend::OpenGLES)
        {
            // Replace header version to 310 es, Otherwise the code above will not work!
            source_code.Replace("#version 300 es", "#version 450");
        }

        const auto compiler = shaderc_compiler_initialize();
        const auto options = shaderc_compile_options_initialize();
        shaderc_compile_options_set_source_language(options, shaderc_source_language_glsl);
        shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level_size);
        shaderc_compile_options_set_target_env(options, shaderc_target_env_opengl, 0);
        shaderc_compile_options_set_auto_map_locations(options, true);
        shaderc_compile_options_set_auto_bind_uniforms(options, true);
        shaderc_compile_options_set_generate_debug_info(options);

        const auto result = shaderc_compile_into_spv(compiler,
            source_code.CString(),
            source_code.Length(),
            get_shader_kind(desc.type),
            desc.name.CString(),
            "main",
            options);

        if(shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success)
        {
            output.error = shaderc_result_get_error_message(result);
            output.has_error = true;
        }
        else
        {
            const auto spirv_code = shaderc_result_get_bytes(result);
            output.error = shaderc_result_get_error_message(result);
            output.spirv_code = ea::vector<u8>(spirv_code, 
                spirv_code + shaderc_result_get_length(result));
        }

        shaderc_result_release(result);
    	shaderc_compile_options_release(options);
        shaderc_compiler_release(compiler);
    }

    void shader_compiler_reflect(const ShaderCompilerReflectDesc& desc, ShaderCompilerReflectInfo& output)
    {
        if (desc.length == 0 || desc.spirv_code == nullptr)
            return;

        spirv_cross::Parser parser{static_cast<uint32_t*>(desc.spirv_code), desc.length / sizeof(uint32_t)};
        parser.parse();
        spirv_cross::Compiler compiler{std::move(parser.get_parsed_ir())};
        auto resources = compiler.get_shader_resources();

        memset(&output.constant_buffer_sizes, 0x0, sizeof(ShaderParameterGroup) * MAX_SHADER_PARAMETER_GROUPS);

        for (const auto& uniform_buffer : resources.uniform_buffers)
        {
            const auto& type = compiler.get_type(uniform_buffer.base_type_id);
            const auto& name = Atomic::String(uniform_buffer.name.c_str());
            const auto& grp_type = utils_get_shader_parameter_group_type(desc.type, name);
            const auto& buffer_size = compiler.get_declared_struct_size(type);

            ShaderCompilerConstantBufferDesc buffer_desc = {};
            buffer_desc.name = name;
            buffer_desc.size = static_cast<uint32_t>(buffer_size);
            buffer_desc.parameter_group = grp_type;
            output.constant_buffers[name] = buffer_desc;

            if (grp_type != Atomic::MAX_SHADER_PARAMETER_GROUPS)
                output.constant_buffer_sizes[grp_type] = buffer_desc.size;

            for (uint32_t i = 0; i < type.member_types.size(); ++i)
            {
                auto member_name = Atomic::String(
                    compiler.get_member_name(uniform_buffer.base_type_id, i).c_str());
                const auto& member_offset = compiler.get_member_decoration(
                    uniform_buffer.base_type_id, i, spv::DecorationOffset);
                const auto& member_size = compiler.get_declared_struct_member_size(type, i);

                if(member_name.Empty())
                    continue;

                // 'c' is a member naming convention on Urho3D/Atomic, we need to get rid this.
                if (member_name.Length() > 0 && member_name[0] == 'c')
                    member_name = member_name.Substring(1);

                if (StringHash(member_name) == Atomic::VSP_MODEL)
                    member_name = member_name;

                Atomic::ShaderParameter shader_param = {};
                shader_param.buffer_ = buffer_desc.parameter_group;
                shader_param.name_ = member_name;
                shader_param.type_ = desc.type;
                shader_param.offset_ = member_offset;
                shader_param.size_ = static_cast<uint32_t>(member_size);

                output.parameters[member_name] = shader_param;
            }
        }

        output.input_elements.resize(resources.stage_inputs.size());
        unsigned idx = 0;
        for (const auto& input : resources.stage_inputs)
        {
            const auto type = compiler.get_type(input.type_id);
            const auto name = Atomic::String(compiler.get_name(input.id).c_str());
            const auto& location = compiler.get_decoration(input.id, spv::DecorationLocation);
            uint8_t semantic_index = 0;
            const auto semantic = utils_get_element_semantic(name, &semantic_index);
            const auto vertex_type = get_element_type(type);

            ShaderCompilerReflectInputElement element = {};
            element.index = static_cast<uint8_t>(location);
            element.semantic_index = semantic_index;
            element.semantic = semantic;
            element.element_type = vertex_type;
            element.name = name;

            if(semantic != MAX_VERTEX_ELEMENT_SEMANTICS)
            {
                output.element_hash <<= 4;
                output.element_hash += (static_cast<int>(semantic) + 1) * semantic_index;
            }

            output.input_elements[idx] = element;
            ++idx;
        }

        idx = 0;

        memset(&output.used_texture_units, 0x0, sizeof(bool) * MAX_TEXTURE_UNITS);
        output.samplers.resize(resources.sampled_images.size());
        for (const auto& image : resources.sampled_images)
        {
            const auto name = ea::string(compiler.get_name(image.id).c_str());
            const auto hash = Atomic::StringHash(compiler.get_name(image.id).c_str());
            const auto& type = compiler.get_type(image.type_id);
            const auto texture_unit = utils_get_texture_unit(name);
            Atomic::TextureUnitType unit_type = Atomic::TextureUnitType::Undefined;

            if (texture_unit != MAX_TEXTURE_UNITS)
                output.used_texture_units[texture_unit] = true;

            switch(type.image.dim)
            {
            case spv::Dim2D:
                unit_type = Atomic::TextureUnitType::Texture2D;
                break;
            case spv::Dim3D:
                unit_type = Atomic::TextureUnitType::Texture3D;
                break;
            case spv::DimCube:
                unit_type = Atomic::TextureUnitType::TextureCube;
                break;
            }

            output.samplers[idx] = { name, hash, texture_unit, unit_type };
            ++idx;
        }
    }

    void shader_compiler_to_hlsl(const ShaderCompilerHlslDesc& desc, Atomic::String& source_code)
    {
#if WIN32
        spirv_cross::CompilerHLSL::Options options = {};
        options.shader_model = 50;
        options.point_size_compat = true;
        HLSLRemapperCompiler compiler(static_cast<uint32_t*>(desc.spirv_code), desc.length / sizeof(uint32_t));
        compiler.set_hlsl_options(options);

        source_code = Atomic::String(compiler.compile().c_str());
#else
        ATOMIC_LOGERROR("HLSL from SpirV is not supported on non windows platform.");
#endif
    }

    struct ShaderFileHeader
    {
        /// Shader version. Used to distinguish shader files between builds
        /// If this header or something in the body changes, you must update
        /// shader version constant
        u32 version{0};
        /// Type of Shader
        ShaderType type{MAX_SHADER_TYPES};
        /// Shader ByteCode Type
        ShaderByteCodeType byte_code_type{ShaderByteCodeType::Max};
        /// Size of ByteCode
        uint32_t byte_code_size{0};
        /// Shader source hash
        uint32_t shader_hash{ 0 };
        /// Size of Strings on Shader File
        uint32_t strings_size{0};
        /// Number of Shader Parameters on Shader File
        uint32_t parameters_count{0};
        /// Number of Textures on Shader File
        uint32_t textures_count{0};
        /// Number of Constant Buffers on Shader File
        uint32_t constant_buffers_count{0};
        /// Number of Input Elements on Shader File
        uint32_t input_elements_count{0};
        /// Input Layout Elements Hash
        uint64_t input_elements_hash{0};
    };

    struct ShaderFileParameter
    {
        uint32_t name_idx{0};
        uint32_t buffer_idx{0};
        uint32_t offset{0};
        uint32_t size{0};
    };

    struct ShaderFileConstantBuffer
    {
        uint32_t name_idx{0};
        uint32_t size{0};
    };

    struct ShaderFileTexture
    {
        u32 name_idx{0};
        TextureUnit unit{MAX_TEXTURE_UNITS};
        TextureUnitType unit_type{TextureUnitType::Undefined};
    };

    struct ShaderFileInputElement
    {
        uint32_t name_idx{0};
        uint8_t index{0};
        uint8_t element_type{0};
        uint8_t semantic_index{0};
        uint8_t semantic{0};
    };

    ea::shared_array<u8> shader_compiler_to_bin(const ShaderCompilerBinDesc& desc, uint32_t* output_length)
    {
        ShaderFileHeader file_header = {};
        file_header.version = SHADER_VERSION;
        file_header.type = desc.type;
        file_header.byte_code_size = desc.byte_code_size;
        file_header.byte_code_type = desc.byte_code_type;
        file_header.shader_hash = desc.shader_hash;
        file_header.parameters_count = desc.reflect_info->parameters.Size();
        file_header.textures_count = desc.reflect_info->samplers.size();
        file_header.constant_buffers_count = desc.reflect_info->constant_buffers.Size();
        file_header.input_elements_count = desc.reflect_info->input_elements.size();
        file_header.input_elements_hash = desc.reflect_info->element_hash;
        constexpr auto header_size = sizeof(ShaderFileHeader);

        // String to Position Map
        ea::unordered_map<u32, uint32_t> str_pos_map = {};

        // Calculate Strings Size and Fill String to Position Map
        for (const auto& it : desc.reflect_info->parameters)
        {
            str_pos_map[it.first_] = file_header.strings_size;
            file_header.strings_size += it.second_.name_.Length() + 1;
        }
        for (const auto& it : desc.reflect_info->samplers)
        {
            str_pos_map[StringHash(it.name.c_str()).ToHash()] = file_header.strings_size;
            file_header.strings_size += it.name.length() + 1;
        }
        for (const auto& it : desc.reflect_info->constant_buffers)
        {
            str_pos_map[it.first_] = file_header.strings_size;
            file_header.strings_size += it.second_.name.Length() + 1;
        }
        for (const auto& it : desc.reflect_info->input_elements)
        {
            str_pos_map[StringHash(it.name).ToHash()] = file_header.strings_size;
            file_header.strings_size += it.name.Length() + 1;
        }
        file_header.strings_size += 1;

        size_t memory_size = header_size + file_header.strings_size;
        memory_size += file_header.parameters_count * sizeof(ShaderFileParameter);
        memory_size += file_header.textures_count * sizeof(ShaderFileTexture);
        memory_size += file_header.constant_buffers_count * sizeof(ShaderFileConstantBuffer);
        memory_size += file_header.input_elements_count * sizeof(ShaderFileInputElement);
        memory_size += file_header.byte_code_size;

        const auto buffer = static_cast<uint8_t*>(malloc(memory_size));
        auto buffer_ptr = buffer;
        size_t str_seek = 0;

        buffer_ptr += header_size;
        const auto str_buffer = static_cast<char*>(static_cast<void*>(buffer_ptr));
        buffer_ptr += file_header.strings_size;

    	const auto file_parameters = static_cast<ShaderFileParameter*>(static_cast<void*>(buffer_ptr));
        buffer_ptr += file_header.parameters_count * sizeof(ShaderFileParameter);

        const auto textures = static_cast<ShaderFileTexture*>(static_cast<void*>(buffer_ptr));
        buffer_ptr += file_header.textures_count * sizeof(ShaderFileTexture);

        const auto constant_buffers = static_cast<ShaderFileConstantBuffer*>(static_cast<void*>(buffer_ptr));
        buffer_ptr += file_header.constant_buffers_count * sizeof(ShaderFileConstantBuffer);

        const auto input_elements = static_cast<ShaderFileInputElement*>(static_cast<void*>(buffer_ptr));
        buffer_ptr += file_header.input_elements_count * sizeof(ShaderFileInputElement);
        const auto byte_code = static_cast<void*>(buffer_ptr);

        // Copy header into memory buffer
        memcpy(buffer, &file_header, header_size);
        memcpy(byte_code, desc.byte_code, desc.byte_code_size);

        uint32_t idx = 0;
        // Copy Shader Parameters into memory buffer
        for (const auto& it : desc.reflect_info->parameters)
        {
            const auto& param = it.second_;
            file_parameters[idx].name_idx = static_cast<uint32_t>(str_seek);
            file_parameters[idx].buffer_idx = param.buffer_;
            file_parameters[idx].offset = param.offset_;
            file_parameters[idx].size = param.size_;

            // Copy name to buffer
            memcpy(str_buffer + str_seek, param.name_.CString(), param.name_.Length()+1);
            str_seek += param.name_.Length() + 1;
            ++idx;
        }

    	// Copy Texture Names into memory buffer
        idx = 0;
        for (const auto& texture : desc.reflect_info->samplers)
        {
            textures[idx].name_idx = static_cast<uint32_t>(str_seek);
            textures[idx].unit = texture.unit;
            textures[idx].unit_type = texture.type;

            memcpy(str_buffer + str_seek, texture.name.c_str(), texture.name.length() + 1);
            str_seek += texture.name.length() + 1;
            ++idx;
        }

        // Copy Constant Buffers into memory buffer
        idx = 0;
        for (const auto& it : desc.reflect_info->constant_buffers)
        {
            constant_buffers[idx].name_idx = static_cast<uint32_t>(str_seek);
            constant_buffers[idx].size = it.second_.size;

            memcpy(str_buffer + str_seek, it.second_.name.CString(), it.second_.name.Length() + 1);
            str_seek += it.second_.name.Length() + 1;
            ++idx;
        }

        // Copy Input Elements into memory buffer
        idx = 0;
        for (const auto& input_element : desc.reflect_info->input_elements)
        {
            input_elements[idx].name_idx = static_cast<uint32_t>(str_seek);
            input_elements[idx].element_type = input_element.element_type;
            input_elements[idx].index = input_element.index;
            input_elements[idx].semantic_index = input_element.semantic_index;
            input_elements[idx].semantic = input_element.semantic;

            memcpy(str_buffer + str_seek, input_element.name.CString(), input_element.name.Length() + 1);
            str_seek += input_element.name.Length() + 1;
            ++idx;
        }

        *output_length = static_cast<uint32_t>(memory_size);
        return ea::shared_array(buffer);
    }

    void shader_compiler_import_bin(void* data, u32 data_size, ShaderCompilerImportBinResult& result)
    {
        constexpr auto header_size = sizeof(ShaderFileHeader);
        if(!data || data_size == 0)
        {
            result.error = "Data pointer is null.";
            result.has_error = true;
            return;
        }
        
        if (data_size < header_size)
        {
            result.error = "Invalid Shader file. Shader file header is invalid.";
            result.has_error = true;
            return;
        }

        auto data_ptr = static_cast<uint8_t*>(data);

        const auto file_header = static_cast<ShaderFileHeader*>(data);
        data_ptr += header_size;

        if(file_header->version != SHADER_VERSION)
        {
            result.error = "Invalid Shader file or shader has been built by other version of engine.";
            result.has_error = true;
            return;
        }

        if(file_header->byte_code_size == 0)
        {
            result.error = "Invalid Shader file or data corrupted. ByteCode size is invalid.";
            result.has_error = true;
            return;
        }
        
        auto expected_data_size = header_size;
        expected_data_size += file_header->strings_size;
        expected_data_size += file_header->byte_code_size;
        expected_data_size += file_header->parameters_count * sizeof(ShaderFileParameter);
        expected_data_size += file_header->textures_count * sizeof(ShaderFileTexture);
        expected_data_size += file_header->constant_buffers_count * sizeof(ShaderFileConstantBuffer);
        expected_data_size += file_header->input_elements_count * sizeof(ShaderFileInputElement);

        if (expected_data_size > data_size)
        {
            result.error = "Invalid Shader file or data corrupted. Expected size is greater than data size.";
            result.has_error = true;
            return;
        }
        
        const auto str_buffer = data_ptr;
        data_ptr += file_header->strings_size;

        const auto parameters = static_cast<ShaderFileParameter*>(static_cast<void*>(data_ptr));
        data_ptr += file_header->parameters_count * sizeof(ShaderFileParameter);

        const auto textures = static_cast<ShaderFileTexture*>(static_cast<void*>(data_ptr));
        data_ptr += file_header->textures_count * sizeof(ShaderFileTexture);

        const auto constant_buffers = static_cast<ShaderFileConstantBuffer*>(static_cast<void*>(data_ptr));
        data_ptr += file_header->constant_buffers_count * sizeof(ShaderFileConstantBuffer);

        const auto input_elements = static_cast<ShaderFileInputElement*>(static_cast<void*>(data_ptr));
        data_ptr += file_header->input_elements_count * sizeof(ShaderFileInputElement);

        const auto byte_code = static_cast<void*>(data_ptr);

        for(uint32_t i =0; i < file_header->parameters_count; ++i)
        {
            const auto& param = parameters[i];
            const auto name = Atomic::String(static_cast<char*>(static_cast<void*>(str_buffer + param.name_idx)));
            ShaderParameter parameter = {};
            parameter.name_ = name;
            parameter.type_ = file_header->type;
            parameter.buffer_ = param.buffer_idx;
            parameter.offset_ = param.offset;
            parameter.size_ = param.size;
            result.reflect_info.parameters[name] = parameter;
        }

        memset(&result.reflect_info.used_texture_units, 0x0, sizeof(bool) * MAX_TEXTURE_UNITS);
        for(uint32_t i =0; i < file_header->textures_count; ++i)
        {
            const auto name = ea::string(static_cast<char*>(static_cast<void*>(str_buffer + textures[i].name_idx)));
            TextureSampler sampler = {
            	name,
            	StringHash(name.c_str()),
            	textures[i].unit,
            	textures[i].unit_type
            };
            result.reflect_info.samplers.push_back(sampler);

            if (textures[i].unit != MAX_TEXTURE_UNITS)
                result.reflect_info.used_texture_units[textures[i].unit] = true;
        }

        memset(&result.reflect_info.constant_buffer_sizes, 0x0, sizeof(uint32_t) * MAX_SHADER_PARAMETER_GROUPS);
        for(uint32_t i =0; i < file_header->constant_buffers_count; ++i)
        {
            const auto& buffer = constant_buffers[i];
            const auto name = Atomic::String(static_cast<char*>(static_cast<void*>(str_buffer + buffer.name_idx)));
            ShaderCompilerConstantBufferDesc buffer_desc = {};
            buffer_desc.name = name;
            buffer_desc.size = buffer.size;
            buffer_desc.parameter_group = utils_get_shader_parameter_group_type(file_header->type, name);
            
            result.reflect_info.constant_buffers[name] = buffer_desc;
            if(buffer_desc.parameter_group == MAX_SHADER_PARAMETER_GROUPS)
                result.reflect_info.constant_buffer_sizes[buffer_desc.parameter_group] = buffer_desc.size;
        }

        result.reflect_info.input_elements.resize(file_header->input_elements_count);
        for(uint32_t i =0; i < file_header->input_elements_count; ++i)
        {
            const auto& input_element = input_elements[i];
            const auto name = Atomic::String(static_cast<char*>(static_cast<void*>(str_buffer + input_element.name_idx)));

            auto& element = result.reflect_info.input_elements[i];
            element.name = name;
            element.index = input_element.index;
            element.element_type = static_cast<VertexElementType>(input_element.element_type);
            element.semantic = static_cast<VertexElementSemantic>(input_element.semantic);
            element.semantic_index = input_element.semantic_index;
        }
        
        result.reflect_info.element_hash = file_header->input_elements_hash;
        result.byte_code = ea::shared_array<u8>(static_cast<uint8_t*>(malloc(file_header->byte_code_size)));
        result.byte_code_size = file_header->byte_code_size;
        result.byte_code_type = file_header->byte_code_type;
        result.shader_hash = file_header->shader_hash;
        result.type = file_header->type;
        result.has_error = false;
        memcpy(result.byte_code.get(), byte_code, file_header->byte_code_size);
    }

    void shader_compiler_get_file_ext(Atomic::ShaderType type, Atomic::String& ext)
    {
        switch (type)
        {
            case Atomic::VS:
                ext = ".vs.rshader";
                break;
            case Atomic::PS:
                ext = ".ps.rshader";
                break;
        }
    }

}
