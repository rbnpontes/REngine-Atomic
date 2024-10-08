#include "../Precompiled.h"

#include "./DiligentUtils.h"
#include "./DriverInstance.h"
#include "./ShaderCompiler.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/Shader.h"
#include "../Graphics/ShaderVariation.h"
#include "../Graphics/VertexBuffer.h"
#include "../IO/File.h"
#include "../IO/FileSystem.h"
#include "../IO/Log.h"
#include "../Resource/ResourceCache.h"

#include "../DebugNew.h"

#include <DiligentCore/Graphics/GraphicsEngine/interface/Shader.h>
#include <DiligentCore/Graphics/GraphicsTools/interface/ShaderMacroHelper.hpp>

namespace Atomic
{
    static const char* s_shader_file_id = "RSHD";
    static ea::array<const char*, static_cast<u32>(GraphicsBackend::Max) + 1> s_shader_backend_name = {
        "D3D11",
        "D3D12",
        "Vk",
        "GL",
        "GLES"
    };

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

        const auto backend = graphics_->GetBackend();
        const String binary_shader_name = graphics_->GetShaderCacheDir()
    	    + String(s_shader_backend_name[static_cast<u32>(backend)])
            + String("/")
    	    + name + "_"
    	    + StringHash(defines_).ToString()
            + extension;

        BuildHash();
        if (!LoadByteCode(binary_shader_name))
        {
            ea::shared_array<u8> shader_file_data;
            u32 shader_file_size = 0;
            // Compile shader if don't have valid bytecode
            if (!Compile(shader_file_data, &shader_file_size)) 
            {
                ATOMIC_LOGERROR(compilerOutput_);
                return false;
            }
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
        byteCode_.clear();
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
        else if((backend == GraphicsBackend::OpenGL || backend == GraphicsBackend::OpenGLES)
                && bin_result.byte_code_type != ShaderByteCodeType::Raw)
        {
            ATOMIC_LOGERROR("Invalid shader bytecode type. Shader file must contains a valid GLSL shader.");
            return false;
        }

        if(hash_ != bin_result.shader_hash)
        {
            ATOMIC_LOGWARNINGF("Invalid shader bytecode for %s. It seems that their original shader was changed or shader bytecode is corrupted!", name_.CString());
            return false;
        }

        byteCode_ = ea::vector<u8>(bin_result.byte_code.get(), bin_result.byte_code.get() + bin_result.byte_code_size);
        elementHash_ = bin_result.reflect_info.element_hash;
        input_elements_ = bin_result.reflect_info.input_elements;
        parameters_ = bin_result.reflect_info.parameters;
        textures_ = bin_result.reflect_info.samplers;
        hash_ = bin_result.shader_hash;

        memcpy(constantBufferSizes_, bin_result.reflect_info.constant_buffer_sizes, sizeof(bool) * MAX_SHADER_PARAMETER_GROUPS);
        memcpy(useTextureUnit_, bin_result.reflect_info.used_texture_units, sizeof(bool) * MAX_TEXTURE_UNITS);

        const auto full_name = GetFullName();
        Diligent::ShaderCreateInfo ci = {};
        ci.Desc.Name =full_name.CString();
        ci.Desc.UseCombinedTextureSamplers = true;
        ci.EntryPoint = "main";
        ci.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_GLSL_VERBATIM;
        
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
            ci.ByteCode = bin_result.byte_code.get();
            ci.ByteCodeSize = bin_result.byte_code_size;
            break;
        case GraphicsBackend::OpenGL:
        case GraphicsBackend::OpenGLES:
            ci.Source = static_cast<char*>(static_cast<void*>(bin_result.byte_code.get()));
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

    bool ShaderVariation::Compile(ea::shared_array<u8>& shader_file_data, u32* shader_file_size)
    {
        const auto full_name = GetFullName();
        Diligent::ShaderCreateInfo shader_ci = {};
        shader_ci.Desc.Name = full_name.CString();
        shader_ci.Desc.UseCombinedTextureSamplers = true;
        shader_ci.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_GLSL_VERBATIM;
        shader_ci.Desc.ShaderType = REngine::utils_get_shader_type(type_);

        String source_code = owner_->GetSourceCode(type_);
        String entrypoint;

        ea::vector<ea::string> defines;
        for (const auto& part : defines_.Split(' '))
            defines.push_back(part.CString());

        const auto backend = graphics_->GetImpl()->GetBackend();

        switch (backend)
        {
        case GraphicsBackend::D3D11:
            defines.push_back("D3D11");
            break;
        case GraphicsBackend::D3D12:
            defines.push_back("D3D12");
            break;
        case GraphicsBackend::Vulkan:
            defines.push_back("VULKAN");
            break;
        case GraphicsBackend::OpenGL:
        case GraphicsBackend::OpenGLES:
	        {
                defines.push_back("OPENGL");
#ifdef ENGINE_PLATFORM_WINDOWS
                if (backend == GraphicsBackend::OpenGLES)
                    defines.push_back("OPENGLES");
#endif
	        }
            break;
        }

        #ifdef ENGINE_PLATFORM_WINDOWS
            defines.push_back("ENGINE_PLATFORM_WINDOWS");
        #endif
        #ifdef ENGINE_PLATFORM_MACOS
            defines.push_back("ENGINE_PLATFORM_MACOS");
        #endif
        #ifdef ENGINE_PLATFORM_IOS
            defines.push_back("ENGINE_PLATFORM_IOS");
        #endif
        #ifdef ENGINE_PLATFORM_APPLE
            defines.push_back("ENGINE_PLATFORM_APPLE");
        #endif
        #ifdef ENGINE_PLATFORM_ANDROID
            defines.push_back("ENGINE_PLATFORM_ANDROID");
        #endif
        
        switch (type_)
        {
        case VS:
            {
                entrypoint = "VS();";
                defines.push_back("COMPILEVS");
            }
            break;
        case PS:
            {
                entrypoint = "PS();";
                defines.push_back("COMPILEPS");
            }
            break;
        case MAX_SHADER_TYPES:
        default:
            ATOMIC_LOGERROR("Invalid Shader type");
            return false;
        }

        defines.push_back(String("MAXBONES=").AppendWithFormat("%d", Graphics::GetMaxBones()).CString());

        // Generate shader macros
        ea::string macros_header;
        for (const auto& define : defines)
        {
            macros_header +="#define ";
            const auto equal_pos = define.find('=');
            if (equal_pos != ea::string::npos)
            {
                macros_header += define.substr(0, equal_pos);
                macros_header += ' ';
                macros_header += define.substr(equal_pos + 1, define.length());
            }
            else
            {
                macros_header += define;
                macros_header += " 1";
            }
            macros_header += '\n';

            // In debug mode, check that all defines are referenced by the shader code
#ifdef ENGINE_DEBUG
            if (source_code.Find(define.c_str()) == String::NPOS)
                ATOMIC_LOGWARNING("Shader " + GetFullName() + " does not use the define " + String(define.c_str()));
#endif
        }

        String glsl_version = "#version 450\n";
#if ENGINE_PLATFORM_MACOS
        glsl_version = "#version 330\n";
#else
        if(backend == GraphicsBackend::OpenGLES)
            glsl_version = "#version 300 es\n";
#endif
        source_code = glsl_version + String(macros_header.c_str()) + source_code;
        source_code.Append("void main()\n{\n");
        source_code.AppendWithFormat("\t%s\n", entrypoint.CString());
        source_code.Append("}");

        REngine::ShaderCompilerReflectInfo reflect_info = {};
        {
            REngine::ShaderCompilerDesc compiler_desc = {};
            compiler_desc.type = type_;
            compiler_desc.backend = backend;
            compiler_desc.name = name_.CString();
            compiler_desc.source_code = source_code;

            // Preprocess shader before compile
            REngine::ShaderCompilerPreProcessResult pre_process_result = {};
            REngine::shader_compiler_preprocess(compiler_desc, pre_process_result);

            if(pre_process_result.has_error)
            {
                compilerOutput_ = pre_process_result.source_code;
                return false;
            }

            // Put pre-processed shader code and compile to obtain spirv bytecode
            compiler_desc.source_code = pre_process_result.source_code;
#ifdef ENGINE_PLATFORM_WINDOWS
            // OpenGL ES on windows is emulated. we must change version to 4.5
            if (backend == GraphicsBackend::OpenGLES)
                compiler_desc.source_code = compiler_desc.source_code.Replaced("#version 300 es", "#version 450");
#endif


            REngine::ShaderCompilerResult result = {};
            REngine::shader_compiler_compile(compiler_desc, true, result);
            if (result.has_error)
            {
                compilerOutput_ = result.error;
                return false;
            }

            REngine::ShaderCompilerReflectDesc reflect_desc = {
                result.spirv_code.data(),
                static_cast<u32>(result.spirv_code.size()),
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
            textures_ = reflect_info.samplers;

#if ENGINE_PLATFORM_WINDOWS
            // On D3D, spirv code needs to be converted to HLSL
            if (backend == GraphicsBackend::D3D11 || backend == GraphicsBackend::D3D12)
            {
                source_code.Clear();
                REngine::shader_compiler_to_hlsl({result.spirv_code.data(), static_cast<u32>(result.spirv_code.size())}, source_code);

                //ATOMIC_LOGDEBUG(source_code);
                shader_ci.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
                shader_ci.Source = source_code.CString();
                shader_ci.SourceLength = source_code.Length();
            }
#endif

            if (backend == GraphicsBackend::Vulkan)
            {
                byteCode_ = result.spirv_code;
                shader_ci.ByteCode = byteCode_.data();
                shader_ci.ByteCodeSize = byteCode_.size();
            }
            else if (backend == GraphicsBackend::OpenGL || backend == GraphicsBackend::OpenGLES)
            {
                const auto byte_code = reinterpret_cast<const unsigned char*>(pre_process_result.source_code.CString());
                byteCode_ = ea::vector<u8>(
                    byte_code,
                    byte_code + pre_process_result.source_code.Length()
                );
                source_code = pre_process_result.source_code;
                shader_ci.Source = source_code.CString();
                shader_ci.SourceLength = source_code.Length();
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
            compilerOutput_ = String(
                static_cast<const char*>(shader_output->GetDataPtr()), 
                static_cast<uint32_t>(shader_output->GetSize())
            );
            return false;
        }

        object_ = shader;

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

#if ENGINE_PLATFORM_WINDOWS
        if (backend == GraphicsBackend::D3D11 || backend == GraphicsBackend::D3D12)
        {
            const void* byte_code = nullptr;
            uint64_t byte_code_len = 0;

            shader->GetBytecode(&byte_code, byte_code_len);

            byteCode_.resize(static_cast<uint32_t>(byte_code_len));
            memcpy(byteCode_.data(), byte_code, sizeof(char) * byte_code_len);
        }
#endif

        REngine::ShaderCompilerBinDesc bin_desc = {};
        bin_desc.byte_code = byteCode_.data();
        bin_desc.byte_code_size = byteCode_.size();
        bin_desc.type = type_;
        bin_desc.shader_hash = hash_;
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
        return true;
    }
    
    void ShaderVariation::SaveByteCode(const String& binaryShaderName, const ea::shared_array<u8>& byte_code,
                                       const u32 byte_code_len) const
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

        const SharedPtr file(new File(owner_->GetContext(), full_name, FILE_WRITE));
        if (!file->IsOpen())
            return;

        file->WriteFileID(s_shader_file_id);
        file->Write(byte_code.get(), byte_code_len);
    }
    void ShaderVariation::BuildHash()
    {
        hash_ = StringHash::Calculate(name_.CString());
        CombineHash(hash_, StringHash::Calculate(GetDefinesClipPlane().CString()));
        CombineHash(hash_, owner_->ToHash());
        CombineHash(hash_, Graphics::GetMaxBones());
        CombineHash(hash_, type_);
    }

}
