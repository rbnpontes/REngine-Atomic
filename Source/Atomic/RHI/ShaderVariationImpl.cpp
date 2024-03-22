#include "../Precompiled.h"

#include "../Graphics/Graphics.h"
#include "../Graphics/Shader.h"
#include "../Graphics/VertexBuffer.h"
#include "../IO/File.h"
#include "../IO/FileSystem.h"
#include "../IO/Log.h"
#include "../Resource/ResourceCache.h"
#include "./DriverInstance.h"

#include "../DebugNew.h"

#include <DiligentCore/Graphics/GraphicsEngine/interface/Shader.h>
#include <DiligentCore/Graphics/GraphicsTools/interface/ShaderMacroHelper.hpp>

namespace Atomic
{

const char* ShaderVariation::elementSemanticNames[] =
{
    "POSITION",
    "NORMAL",
    "BINORMAL",
    "TANGENT",
    "TEXCOORD",
    "COLOR",
    "BLENDWEIGHT",
    "BLENDINDICES",
    "OBJECTINDEX"
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
    extension = type_ == VS ? ".vs4" : ".ps4";

    const String binary_shader_name = graphics_->GetShaderCacheDir() + name + "_" + StringHash(defines_).ToString() + extension;
    
    if (!LoadByteCode(binary_shader_name))
    {
        // Compile shader if don't have valid bytecode
        if (!Compile())
            return false;
        // Save the bytecode after successful compile, but not if the source is from a package
        if (owner_->GetTimeStamp())
            SaveByteCode(binary_shader_name);
    }

    Diligent::ShaderCreateInfo ci = {};
    ci.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
    ci.ByteCode = byteCode_.Buffer();
    ci.ByteCodeSize = byteCode_.Size();

    switch (type_)
    {
    case VS:
        ci.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
        break;
    case PS:
        ci.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        break;
    default:
        compilerOutput_ = "Could not create shader. Invalid shader type!";
        return nullptr;
    }

    Diligent::RefCntAutoPtr<Diligent::IShader> shader;
    Diligent::RefCntAutoPtr<Diligent::IDataBlob> shader_output;

    graphics_->GetImpl()->GetDevice()->CreateShader(ci, &shader, &shader_output);

    if(!shader)
    {
        compilerOutput_ = "Could not create shader.";
        const String compiler_message(static_cast<const char*>(shader_output->GetDataPtr()), shader_output->GetSize());
        if(compiler_message.Length())
        {
            compilerOutput_.Append("\nOutput: ");
            compilerOutput_.Append(compiler_message);
        }
        
        return nullptr;
    }

    object_ = shader;
    return true;
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
    
    FileSystem* fileSystem = owner_->GetSubsystem<FileSystem>();
    unsigned sourceTimeStamp = owner_->GetTimeStamp();
    // If source code is loaded from a package, its timestamp will be zero. Else check that binary is not older
    // than source
    if (sourceTimeStamp && fileSystem->GetLastModifiedTime(cache->GetResourceFileName(binaryShaderName)) < sourceTimeStamp)
        return false;
    
    SharedPtr<File> file = cache->GetFile(binaryShaderName);
    if (!file || file->ReadFileID() != "USHD")
    {
        ATOMIC_LOGERROR(binaryShaderName + " is not a valid shader bytecode file");
        return false;
    }
    
    /// \todo Check that shader type and model match
    /*unsigned short shaderType = */file->ReadUShort();
    /*unsigned short shaderModel = */file->ReadUShort();
    elementHash_ = file->ReadUInt();
    elementHash_ <<= 32;
    
    unsigned numParameters = file->ReadUInt();
    for (unsigned i = 0; i < numParameters; ++i)
    {
        String name = file->ReadString();
        unsigned buffer = file->ReadUByte();
        unsigned offset = file->ReadUInt();
        unsigned size = file->ReadUInt();
    
        ShaderParameter parameter;
        parameter.type_ = type_;
        parameter.name_ = name;
        parameter.buffer_ = buffer;
        parameter.offset_ = offset;
        parameter.size_ = size;
        parameters_[StringHash(name)] = parameter;
    }
    
    unsigned numTextureUnits = file->ReadUInt();
    for (unsigned i = 0; i < numTextureUnits; ++i)
    {
        /*String unitName = */file->ReadString();
        unsigned reg = file->ReadUByte();
    
        if (reg < MAX_TEXTURE_UNITS)
            useTextureUnit_[reg] = true;
    }
    
    unsigned byteCodeSize = file->ReadUInt();
    if (byteCodeSize)
    {
        byteCode_.Resize(byteCodeSize);
        file->Read(&byteCode_[0], byteCodeSize);
    
        if (type_ == VS)
            ATOMIC_LOGDEBUG("Loaded cached vertex shader " + GetFullName());
        else
            ATOMIC_LOGDEBUG("Loaded cached pixel shader " + GetFullName());
    
        CalculateConstantBufferSizes();
        return true;
    }
    else
    {
        ATOMIC_LOGERROR(binaryShaderName + " has zero length bytecode");
        return false;
    }
}

bool ShaderVariation::Compile()
{
    Diligent::ShaderCreateInfo shader_ci = {};
    Diligent::ShaderMacroHelper macros = {};
    
    const String& source_code = owner_->GetSourceCode(type_);
    Vector<String> defines = defines_.Split(' ');
    const auto backend = graphics_->GetImpl()->GetBackend();

    switch (backend)
    {
    case GraphicsBackend::D3D11:
        macros.Add("D3D11", "1");
        break;
    case GraphicsBackend::D3D12:
        macros.Add("D3D12", "1");
        break;
    case GraphicsBackend::Vulkan:
        macros.Add("VULKAN", "1");
        break;
    case GraphicsBackend::OpenGL:
        macros.Add("OPENGL", "1");
        break;
    }

    switch (type_)
    {
        case VS:
        {
            shader_ci.EntryPoint = "VS";
            macros.Add("COMPILEVS", "1");
        }
        break;
        case PS:
        {
            shader_ci.EntryPoint = "PS";
            macros.Add("COMPILEPS", "1");
        }
        break;
    }

    macros.Add("MAXBONES", String(Graphics::GetMaxBones()).CString());

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
    
    for (unsigned i = 0; i < defines.Size(); ++i)
    {
        macros.Add(defines[i].CString(), define_values[i].CString());

        // In debug mode, check that all defines are referenced by the shader code
#ifdef _DEBUG
        if (source_code.Find(defines[i]) == String::NPOS)
            ATOMIC_LOGWARNING("Shader " + GetFullName() + " does not use the define " + defines[i]);
#endif
    }

    shader_ci.Macros = macros;
    shader_ci.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
    shader_ci.SourceLength = source_code.Length();
    shader_ci.Source = source_code.CString();

    Diligent::RefCntAutoPtr<Diligent::IShader> shader;
    Diligent::RefCntAutoPtr<Diligent::IDataBlob> shader_output;

    graphics_->GetImpl()->GetDevice()->CreateShader(shader_ci,
        &shader,
        &shader_output);

    if(!shader)
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
    }

    const void* byte_code = nullptr;
    uint64_t byte_code_len = 0;
    
    shader->GetBytecode(&byte_code, byte_code_len);
    return true;
    // Compile using D3DCompile
    // ID3DBlob* shaderCode = 0;
    // ID3DBlob* errorMsgs = 0;
    //
    // HRESULT hr = D3DCompile(source_code.CString(), source_code.Length(), owner_->GetName().CString(), &macros.Front(), 0,
    //     entry_point, profile, flags, 0, &shaderCode, &errorMsgs);
    // if (FAILED(hr))
    // {
    //     // Do not include end zero unnecessarily
    //     compilerOutput_ = String((const char*)errorMsgs->GetBufferPointer(), (unsigned)errorMsgs->GetBufferSize() - 1);
    // }
    // else
    // {
    //     if (type_ == VS)
    //         ATOMIC_LOGDEBUG("Compiled vertex shader " + GetFullName());
    //     else
    //         ATOMIC_LOGDEBUG("Compiled pixel shader " + GetFullName());
    //
    //     unsigned char* bufData = (unsigned char*)shaderCode->GetBufferPointer();
    //     unsigned bufSize = (unsigned)shaderCode->GetBufferSize();
    //     // Use the original bytecode to reflect the parameters
    //     ParseParameters(bufData, bufSize);
    //     CalculateConstantBufferSizes();
    //
    //     // Then strip everything not necessary to use the shader
    //     ID3DBlob* strippedCode = 0;
    //     D3DStripShader(bufData, bufSize,
    //         D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO | D3DCOMPILER_STRIP_TEST_BLOBS, &strippedCode);
    //     byteCode_.Resize((unsigned)strippedCode->GetBufferSize());
    //     memcpy(&byteCode_[0], strippedCode->GetBufferPointer(), byteCode_.Size());
    //     strippedCode->Release();
    // }
    //
    // ATOMIC_SAFE_RELEASE(shaderCode);
    // ATOMIC_SAFE_RELEASE(errorMsgs);
    //
    // return !byteCode_.Empty();
}

void ShaderVariation::ParseParameters(unsigned char* bufData, unsigned bufSize)
{
    throw std::exception("Not implemented");
    // ID3D11ShaderReflection* reflection = 0;
    // D3D11_SHADER_DESC shaderDesc;
    //
    // HRESULT hr = D3DReflect(bufData, bufSize, IID_ID3D11ShaderReflection, (void**)&reflection);
    // if (FAILED(hr) || !reflection)
    // {
    //     ATOMIC_SAFE_RELEASE(reflection);
    //     ATOMIC_LOGD3DERROR("Failed to reflect vertex shader's input signature", hr);
    //     return;
    // }
    //
    // reflection->GetDesc(&shaderDesc);
    //
    // if (type_ == VS)
    // {
    //     for (unsigned i = 0; i < shaderDesc.InputParameters; ++i)
    //     {
    //         D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
    //         reflection->GetInputParameterDesc((UINT)i, &paramDesc);
    //         VertexElementSemantic semantic = (VertexElementSemantic)GetStringListIndex(paramDesc.SemanticName, elementSemanticNames, MAX_VERTEX_ELEMENT_SEMANTICS, true);
    //         if (semantic != MAX_VERTEX_ELEMENT_SEMANTICS)
    //         {
    //             elementHash_ <<= 4;
    //             elementHash_ += ((int)semantic + 1) * (paramDesc.SemanticIndex + 1);
    //         }
    //     }
    //     elementHash_ <<= 32;
    // }
    //
    // HashMap<String, unsigned> cbRegisterMap;
    //
    // for (unsigned i = 0; i < shaderDesc.BoundResources; ++i)
    // {
    //     D3D11_SHADER_INPUT_BIND_DESC resourceDesc;
    //     reflection->GetResourceBindingDesc(i, &resourceDesc);
    //     String resourceName(resourceDesc.Name);
    //     if (resourceDesc.Type == D3D_SIT_CBUFFER)
    //         cbRegisterMap[resourceName] = resourceDesc.BindPoint;
    //     else if (resourceDesc.Type == D3D_SIT_SAMPLER && resourceDesc.BindPoint < MAX_TEXTURE_UNITS)
    //         useTextureUnit_[resourceDesc.BindPoint] = true;
    // }
    //
    // for (unsigned i = 0; i < shaderDesc.ConstantBuffers; ++i)
    // {
    //     ID3D11ShaderReflectionConstantBuffer* cb = reflection->GetConstantBufferByIndex(i);
    //     D3D11_SHADER_BUFFER_DESC cbDesc;
    //     cb->GetDesc(&cbDesc);
    //     unsigned cbRegister = cbRegisterMap[String(cbDesc.Name)];
    //
    //     for (unsigned j = 0; j < cbDesc.Variables; ++j)
    //     {
    //         ID3D11ShaderReflectionVariable* var = cb->GetVariableByIndex(j);
    //         D3D11_SHADER_VARIABLE_DESC varDesc;
    //         var->GetDesc(&varDesc);
    //         String varName(varDesc.Name);
    //         if (varName[0] == 'c')
    //         {
    //             varName = varName.Substring(1); // Strip the c to follow Urho3D constant naming convention
    //             ShaderParameter parameter;
    //             parameter.type_ = type_;
    //             parameter.name_ = varName;
    //             parameter.buffer_ = cbRegister;
    //             parameter.offset_ = varDesc.StartOffset;
    //             parameter.size_ = varDesc.Size;
    //             parameters_[varName] = parameter;
    //         }
    //     }
    // }
    //
    // reflection->Release();
}

void ShaderVariation::SaveByteCode(const String& binaryShaderName)
{
    throw std::exception("Not implemented");
    // ResourceCache* cache = owner_->GetSubsystem<ResourceCache>();
    // FileSystem* fileSystem = owner_->GetSubsystem<FileSystem>();
    //
    // // Filename may or may not be inside the resource system
    // String fullName = binaryShaderName;
    // if (!IsAbsolutePath(fullName))
    // {
    //     // If not absolute, use the resource dir of the shader
    //     String shaderFileName = cache->GetResourceFileName(owner_->GetName());
    //     if (shaderFileName.Empty())
    //         return;
    //     fullName = shaderFileName.Substring(0, shaderFileName.Find(owner_->GetName())) + binaryShaderName;
    // }
    // String path = GetPath(fullName);
    // if (!fileSystem->DirExists(path))
    //     fileSystem->CreateDir(path);
    //
    // SharedPtr<File> file(new File(owner_->GetContext(), fullName, FILE_WRITE));
    // if (!file->IsOpen())
    //     return;
    //
    // file->WriteFileID("USHD");
    // file->WriteShort((unsigned short)type_);
    // file->WriteShort(4);
    // file->WriteUInt(elementHash_ >> 32);
    //
    // file->WriteUInt(parameters_.Size());
    // for (HashMap<StringHash, ShaderParameter>::ConstIterator i = parameters_.Begin(); i != parameters_.End(); ++i)
    // {
    //     file->WriteString(i->second_.name_);
    //     file->WriteUByte((unsigned char)i->second_.buffer_);
    //     file->WriteUInt(i->second_.offset_);
    //     file->WriteUInt(i->second_.size_);
    // }
    //
    // unsigned usedTextureUnits = 0;
    // for (unsigned i = 0; i < MAX_TEXTURE_UNITS; ++i)
    // {
    //     if (useTextureUnit_[i])
    //         ++usedTextureUnits;
    // }
    // file->WriteUInt(usedTextureUnits);
    // for (unsigned i = 0; i < MAX_TEXTURE_UNITS; ++i)
    // {
    //     if (useTextureUnit_[i])
    //     {
    //         file->WriteString(graphics_->GetTextureUnitName((TextureUnit)i));
    //         file->WriteUByte((unsigned char)i);
    //     }
    // }
    //
    // file->WriteUInt(byteCode_.Size());
    // if (byteCode_.Size())
    //     file->Write(&byteCode_[0], byteCode_.Size());
}

void ShaderVariation::CalculateConstantBufferSizes()
{
    throw std::exception("Not implemented");
    // for (unsigned i = 0; i < MAX_SHADER_PARAMETER_GROUPS; ++i)
    //     constantBufferSizes_[i] = 0;
    //
    // for (HashMap<StringHash, ShaderParameter>::ConstIterator i = parameters_.Begin(); i != parameters_.End(); ++i)
    // {
    //     if (i->second_.buffer_ < MAX_SHADER_PARAMETER_GROUPS)
    //     {
    //         unsigned oldSize = constantBufferSizes_[i->second_.buffer_];
    //         unsigned paramEnd = i->second_.offset_ + i->second_.size_;
    //         if (paramEnd > oldSize)
    //             constantBufferSizes_[i->second_.buffer_] = paramEnd;
    //     }
    // }
}

}
