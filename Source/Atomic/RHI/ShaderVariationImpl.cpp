#include "../Precompiled.h"

#include "../Graphics/Graphics.h"
#include "../Graphics/Shader.h"
#include "../Graphics/VertexBuffer.h"
#include "../Graphics/ShaderVariation.h"
#include "../IO/File.h"
#include "../IO/FileSystem.h"
#include "../IO/Log.h"
#include "../Resource/ResourceCache.h"
#include "./DriverInstance.h"
#include "./ShaderCompiler.h"
#include "./DiligentUtils.h"

#include "../DebugNew.h"

#include <DiligentCore/Graphics/GraphicsEngine/interface/Shader.h>
#include <DiligentCore/Graphics/GraphicsTools/interface/ShaderMacroHelper.hpp>

namespace Atomic
{
    static const char* s_shader_file_id = "RSHD";
    void ShaderVariation::OnDeviceLost()
    {
        // No-op on Direct3D11
    }

    bool ShaderVariation::Create()
    {
        Release();

        if (!graphics_)
            return false;

        if (!owner_)
        {
            compilerOutput_ = "Owner shader has expired";
            return false;
        }

        // Check for up-to-date bytecode on disk
        String path, name, extension;
        SplitPath(owner_->GetName(), path, name, extension);
        REngine::shader_compiler_get_file_ext(type_, extension);

        const String binary_shader_name = graphics_->GetShaderCacheDir() + name + "_" + StringHash(defines_).ToString()
            + extension;

        if (!LoadByteCode(binary_shader_name))
        {
            SharedArrayPtr<uint8_t> shader_file_data;
            uint32_t shader_file_size = 0;
            // Compile shader if don't have valid bytecode
            if (!Compile(shader_file_data, &shader_file_size))
                return false;
            // Save Shader File
            SaveByteCode(binary_shader_name, shader_file_data, shader_file_size);
        }

        return object_ != nullptr;
    }

    void ShaderVariation::Release()
    {
        if (!graphics_)
            return;
        if (object_)
        {
            graphics_->CleanupShaderPrograms(this);

            if (type_ == VS)
            {
                if (graphics_->GetVertexShader() == this)
                    graphics_->SetShaders(nullptr, nullptr);
            }
            else
            {
                if (graphics_->GetPixelShader() == this)
                    graphics_->SetShaders(nullptr, nullptr);
            }

            object_ = nullptr;
        }

        compilerOutput_.Clear();

        for (bool& i : useTextureUnit_)
            i = false;
        for (unsigned int& constant_buffer_size : constantBufferSizes_)
            constant_buffer_size = 0;
        parameters_.Clear();
        byteCode_.Clear();
        elementHash_ = 0;
    }

    void ShaderVariation::SetDefines(const String& defines)
    {
        defines_ = defines;

        // Internal mechanism for appending the CLIPPLANE define, prevents runtime (every frame) string manipulation
        definesClipPlane_ = defines;
        if (!definesClipPlane_.EndsWith(" CLIPPLANE"))
            definesClipPlane_ += " CLIPPLANE";
    }

    bool ShaderVariation::LoadByteCode(const String& binaryShaderName)
    {
        ResourceCache* cache = owner_->GetSubsystem<ResourceCache>();
        if (!cache->Exists(binaryShaderName))
            return false;

        /*const auto file_system = owner_->GetSubsystem<FileSystem>();
        const auto source_time_stamp = owner_->GetTimeStamp();*/
        // If source code is loaded from a package, its timestamp will be zero. Else check that binary is not older
        // than source
        // TODO: fix this
       /* if (source_time_stamp && file_system->GetLastModifiedTime(cache->GetResourceFileName(binaryShaderName)) <
            source_time_stamp)
            return false;*/

        const SharedPtr<File> file = cache->GetFile(binaryShaderName);
        if (!file || file->ReadFileID() != s_shader_file_id)
        {
            ATOMIC_LOGERROR(binaryShaderName + " is not a valid shader bytecode file");
            return false;
        }

        const auto file_size = file->GetSize() - 4;
        if(file_size == 0)
        {
            ATOMIC_LOGERROR(binaryShaderName + " has zero bytes");
            return false;
        }

        const SharedArrayPtr<uint8_t> byte_code(static_cast<uint8_t*>(malloc(file_size)));
        file->Read(byte_code.Get(), file_size);

        REngine::ShaderCompilerImportBinResult bin_result = {};
        REngine::shader_compiler_import_bin(byte_code, file_size, bin_result);

        const auto backend = graphics_->GetImpl()->GetBackend();
        if(backend == GraphicsBackend::D3D11 || backend == GraphicsBackend::D3D12)
        {
            if(bin_result.byte_code_type != ShaderByteCodeType::DxB)
            {
                ATOMIC_LOGERROR("Invalid shader bytecode type. Compiled shader file is not compatible with DirectX bytecode.");
                return false;
            }
        }
        else if(backend == GraphicsBackend::Vulkan && bin_result.byte_code_type != ShaderByteCodeType::SpirV)
        {
            ATOMIC_LOGERROR("Invalid shader bytecode type. Compiled shader file is not compatible with Vulkan bytecode.");
            return false;  
        }
        else if(backend == GraphicsBackend::OpenGL && bin_result.byte_code_type != ShaderByteCodeType::Raw)
        {
            ATOMIC_LOGERROR("Invalid shader bytecode type. Shader file must contains a valid GLSL shader.");
            return false;
        }
        
        byteCode_ = PODVector<uint8_t>(bin_result.byte_code, bin_result.byte_code_size);
        elementHash_ = bin_result.reflect_info.element_hash;
        input_elements_ = bin_result.reflect_info.input_elements;
        parameters_ = bin_result.reflect_info.parameters;
        used_textures_ = bin_result.reflect_info.samplers;

        memcpy(constantBufferSizes_, bin_result.reflect_info.constant_buffer_sizes, sizeof(bool) * MAX_SHADER_PARAMETER_GROUPS);
        memcpy(useTextureUnit_, bin_result.reflect_info.used_texture_units, sizeof(bool) * MAX_TEXTURE_UNITS);

        const auto full_name = GetFullName();
        Diligent::ShaderCreateInfo ci;
        ci.Desc.Name =full_name.CString();
        ci.EntryPoint = "main";
        ci.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_GLSL;
        
        switch (type_)
        {
        case VS:
            ci.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
            break;
        case PS:
            ci.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
            break;
        }

        switch (backend)
        {
        case GraphicsBackend::D3D11:
        case GraphicsBackend::D3D12:
        case GraphicsBackend::Vulkan:
            ci.ByteCode = bin_result.byte_code;
            ci.ByteCodeSize = bin_result.byte_code_size;
            break;
        case GraphicsBackend::OpenGL:
            ci.Source = static_cast<char*>(static_cast<void*>(bin_result.byte_code));
            ci.SourceLength = bin_result.byte_code_size;
            break;
        }

        Diligent::IShader* shader = nullptr;
        graphics_->GetImpl()->GetDevice()->CreateShader(ci, &shader, nullptr);

        if(!shader)
        {
            ATOMIC_LOGERROR("Failed to create shader from Shader File");
            return false;
        }

        object_ = shader;
        return true;
    }

    bool ShaderVariation::Compile(SharedArrayPtr<uint8_t>& shader_file_data, uint32_t* shader_file_size)
    {
        const auto full_name = GetFullName();
        Diligent::ShaderCreateInfo shader_ci = {};
        shader_ci.Desc.Name = full_name.CString();
        shader_ci.Desc.UseCombinedTextureSamplers = true;
        shader_ci.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_GLSL;
        shader_ci.Desc.ShaderType = REngine::utils_get_shader_type(type_);

        String source_code = owner_->GetSourceCode(type_);
        String entrypoint;
        Vector<String> defines = defines_.Split(' ');
        const auto backend = graphics_->GetImpl()->GetBackend();

        switch (backend)
        {
        case GraphicsBackend::D3D11:
            defines.Push("D3D11");
            break;
        case GraphicsBackend::D3D12:
            defines.Push("D3D12");
            break;
        case GraphicsBackend::Vulkan:
            defines.Push("VULKAN");
            break;
        case GraphicsBackend::OpenGL:
            defines.Push("OPENGL");
            break;
        }


        switch (type_)
        {
        case VS:
            {
                entrypoint = "VS();";
                defines.Push("COMPILEVS");
            }
            break;
        case PS:
            {
                entrypoint = "PS();";
                defines.Push("COMPILEPS");
            }
            break;
        case MAX_SHADER_TYPES:
        default:
            ATOMIC_LOGERROR("Invalid Shader type");
            return false;
        }

        defines.Push(String("MAXBONES=") + String(Graphics::GetMaxBones()).CString());

        // Collect defines into macros
        Vector<String> define_values;
        for (unsigned i = 0; i < defines.Size(); ++i)
        {
            unsigned equalsPos = defines[i].Find('=');
            if (equalsPos != String::NPOS)
            {
                define_values.Push(defines[i].Substring(equalsPos + 1));
                defines[i].Resize(equalsPos);
            }
            else
                define_values.Push("1");
        }

        String macros_header;
        for (unsigned i = 0; i < defines.Size(); ++i)
        {
            macros_header.Append("#define ");
            const auto equal_pos = defines[i].Find('=');
            if (equal_pos != String::NPOS)
            {
                macros_header.Append(defines[i].Substring(equal_pos + 1));
                macros_header.Append(' ');
                macros_header.Append(defines[i].Substring(equal_pos + 1, defines[i].Length()));
            }
            else
            {
                macros_header.Append(defines[i]);
                macros_header.Append(" 1");
            }
            macros_header.Append('\n');

            // In debug mode, check that all defines are referenced by the shader code
#ifdef _DEBUG
            if (source_code.Find(defines[i]) == String::NPOS)
                ATOMIC_LOGWARNING("Shader " + GetFullName() + " does not use the define " + defines[i]);
#endif
        }

        source_code = String("#version 450\n") + macros_header + source_code;
        source_code.Append("void main()\n{");
        source_code.AppendWithFormat("\t%s\n", entrypoint.CString());
        source_code.Append("}");

        REngine::ShaderCompilerReflectInfo reflect_info = {};
        {
            REngine::ShaderCompilerDesc compiler_desc = {};
            compiler_desc.type = type_;
            compiler_desc.source_code = source_code;
            REngine::ShaderCompilerResult result = {};

            REngine::shader_compiler_compile(compiler_desc, true, result);
            if (result.has_error)
            {
                ATOMIC_LOGERROR(result.error);
                compilerOutput_ = result.error;
                return false;
            }

//#if defined(DEBUG) || defined(_DEBUG)
//            {
//                REngine::ShaderCompilerPreProcessResult pre_process_result = {};
//                REngine::shader_compiler_preprocess(compiler_desc, pre_process_result);
//
//                String defines_txt;
//                for(unsigned i = 0; i < defines.Size(); ++i)
//                {
//					defines_txt.Append(defines[i]);
//                    if(i < defines.Size() - 1)
//						defines_txt.Append(" ");
//				}
//                ATOMIC_LOGDEBUGF("BEGIN PREPROCESS %s", shader_ci.Desc.Name);
//                ATOMIC_LOGDEBUGF("Defines: %s", defines_txt.CString());
//                ATOMIC_LOGDEBUG(pre_process_result.source_code);
//                ATOMIC_LOGDEBUGF("END PREPROCESS %s", shader_ci.Desc.Name);
//            }
//#endif

            REngine::ShaderCompilerReflectDesc reflect_desc = {
                result.spirv_code.Buffer(),
                result.spirv_code.Size(),
                type_
            };
            REngine::shader_compiler_reflect(reflect_desc, reflect_info);

            for (uint8_t i = 0; i < MAX_TEXTURE_UNITS; ++i)
                useTextureUnit_[i] = reflect_info.used_texture_units[i];
            for (uint8_t i = 0; i < MAX_SHADER_PARAMETER_GROUPS; ++i)
                constantBufferSizes_[i] = reflect_info.constant_buffer_sizes[i];

            elementHash_ = reflect_info.element_hash;
            parameters_ = reflect_info.parameters;
            input_elements_ = reflect_info.input_elements;
            used_textures_ = reflect_info.samplers;
#if WIN32
            // On D3D, spirv code needs to be converted to HLSL
            if (backend == GraphicsBackend::D3D11 || backend == GraphicsBackend::D3D12)
            {
                source_code.Clear();
                REngine::shader_compiler_to_hlsl({result.spirv_code.Buffer(), result.spirv_code.Size()}, source_code);

                shader_ci.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
                shader_ci.Source = source_code.CString();
                shader_ci.SourceLength = source_code.Length();
            }
#endif
            if (backend == GraphicsBackend::Vulkan)
            {
                byteCode_ = result.spirv_code;
                shader_ci.ByteCode = byteCode_.Buffer();
                shader_ci.ByteCodeSize = byteCode_.Size();
            }
            else if (backend == GraphicsBackend::OpenGL)
            {
                REngine::ShaderCompilerPreProcessResult pre_process_result = {};
                REngine::shader_compiler_preprocess(compiler_desc, pre_process_result);
                const auto byte_code = reinterpret_cast<const unsigned char*>(pre_process_result.source_code.CString());
                byteCode_ = PODVector<unsigned char>(
                    byte_code,
                    pre_process_result.source_code.Length());
                shader_ci.EntryPoint = "main";
            }
        }

        Diligent::RefCntAutoPtr<Diligent::IShader> shader;
        Diligent::RefCntAutoPtr<Diligent::IDataBlob> shader_output;

        graphics_->GetImpl()->GetDevice()->CreateShader(shader_ci,
                                                        &shader,
                                                        &shader_output);

        if (!shader)
        {
            compilerOutput_ = String(static_cast<const char*>(shader_output->GetDataPtr()), shader_output->GetSize());
            return false;
        }

        switch (type_)
        {
        case VS:
            ATOMIC_LOGDEBUG("Compiled vertex shader " + GetFullName());
            break;
        case PS:
            ATOMIC_LOGDEBUG("Compiled pixel shader " + GetFullName());
            break;
        case MAX_SHADER_TYPES:
        default:
            ATOMIC_LOGERROR("Unknown shader type");
            return false;
        }

#if WIN32
        if (backend == GraphicsBackend::D3D11 || backend == GraphicsBackend::D3D12)
        {
            const void* byte_code = nullptr;
            uint64_t byte_code_len = 0;

            shader->GetBytecode(&byte_code, byte_code_len);

            byteCode_.Resize(static_cast<uint32_t>(byte_code_len));
            memcpy(byteCode_.Buffer(), byte_code, sizeof(char) * byte_code_len);
        }
#endif

        REngine::ShaderCompilerBinDesc bin_desc = {};
        bin_desc.byte_code = byteCode_.Buffer();
        bin_desc.byte_code_size = byteCode_.Size();
        bin_desc.type = type_;
        bin_desc.reflect_info = &reflect_info;

        if (backend == GraphicsBackend::D3D11 || backend == GraphicsBackend::D3D12)
            bin_desc.byte_code_type = ShaderByteCodeType::DxB;
        else if (backend == GraphicsBackend::Vulkan)
            bin_desc.byte_code_type = ShaderByteCodeType::SpirV;
        else
            bin_desc.byte_code_type = ShaderByteCodeType::Raw;

        uint32_t bin_length = 0;
        auto bin_data = REngine::shader_compiler_to_bin(bin_desc, &bin_length);

        shader_file_data = bin_data;
        *shader_file_size = bin_length;

        object_ = shader;
        return true;
    }

    void ShaderVariation::SaveByteCode(const String& binaryShaderName, const SharedArrayPtr<uint8_t>& byte_code,
                                       const uint32_t byte_code_len) const
    {
        const auto cache = owner_->GetSubsystem<ResourceCache>();
        const auto fileSystem = owner_->GetSubsystem<FileSystem>();

        // Filename may or may not be inside the resource system
        auto full_name = binaryShaderName;
        if (!IsAbsolutePath(full_name))
        {
            // If not absolute, use the resource dir of the shader
            const auto shader_file_name = cache->GetResourceFileName(owner_->GetName());
            if (shader_file_name.Empty())
                return;
            full_name = shader_file_name.Substring(0, shader_file_name.Find(owner_->GetName())) + binaryShaderName;
        }
        
        const auto path = GetPath(full_name);
        if (!fileSystem->DirExists(path))
            fileSystem->CreateDir(path);

        SharedPtr<File> file(new File(owner_->GetContext(), full_name, FILE_WRITE));
        if (!file->IsOpen())
            return;

        file->WriteFileID(s_shader_file_id);
        file->Write(byte_code.Get(), byte_code_len);
    }
}
