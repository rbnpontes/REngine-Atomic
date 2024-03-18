#include "../Precompiled.h"

#include "../Graphics/Graphics.h"
#include "../Graphics/Shader.h"
#include "../Graphics/VertexBuffer.h"
#include "../IO/File.h"
#include "../IO/FileSystem.h"
#include "../IO/Log.h"
#include "../Resource/ResourceCache.h"

#include "../DebugNew.h"

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
    throw std::exception("Not implemented");
    // Release();
    //
    // if (!graphics_)
    //     return false;
    //
    // if (!owner_)
    // {
    //     compilerOutput_ = "Owner shader has expired";
    //     return false;
    // }
    //
    // // Check for up-to-date bytecode on disk
    // String path, name, extension;
    // SplitPath(owner_->GetName(), path, name, extension);
    // extension = type_ == VS ? ".vs4" : ".ps4";
    //
    // String binaryShaderName = graphics_->GetShaderCacheDir() + name + "_" + StringHash(defines_).ToString() + extension;
    //
    // if (!LoadByteCode(binaryShaderName))
    // {
    //     // Compile shader if don't have valid bytecode
    //     if (!Compile())
    //         return false;
    //     // Save the bytecode after successful compile, but not if the source is from a package
    //     if (owner_->GetTimeStamp())
    //         SaveByteCode(binaryShaderName);
    // }
    //
    // // Then create shader from the bytecode
    // ID3D11Device* device = graphics_->GetImpl()->GetDevice();
    // if (type_ == VS)
    // {
    //     if (device && byteCode_.Size())
    //     {
    //         HRESULT hr = device->CreateVertexShader(&byteCode_[0], byteCode_.Size(), 0, (ID3D11VertexShader**)&object_.ptr_);
    //         if (FAILED(hr))
    //         {
    //             ATOMIC_SAFE_RELEASE(object_.ptr_);
    //             compilerOutput_ = "Could not create vertex shader (HRESULT " + ToStringHex((unsigned)hr) + ")";
    //         }
    //     }
    //     else
    //         compilerOutput_ = "Could not create vertex shader, empty bytecode";
    // }
    // else
    // {
    //     if (device && byteCode_.Size())
    //     {
    //         HRESULT hr = device->CreatePixelShader(&byteCode_[0], byteCode_.Size(), 0, (ID3D11PixelShader**)&object_.ptr_);
    //         if (FAILED(hr))
    //         {
    //             ATOMIC_SAFE_RELEASE(object_.ptr_);
    //             compilerOutput_ = "Could not create pixel shader (HRESULT " + ToStringHex((unsigned)hr) + ")";
    //         }
    //     }
    //     else
    //         compilerOutput_ = "Could not create pixel shader, empty bytecode";
    // }
    //
    // return object_.ptr_ != 0;
}

void ShaderVariation::Release()
{
    throw std::exception("Not implemented");
    // if (object_.ptr_)
    // {
    //     if (!graphics_)
    //         return;
    //
    //     graphics_->CleanupShaderPrograms(this);
    //
    //     if (type_ == VS)
    //     {
    //         if (graphics_->GetVertexShader() == this)
    //             graphics_->SetShaders(0, 0);
    //     }
    //     else
    //     {
    //         if (graphics_->GetPixelShader() == this)
    //             graphics_->SetShaders(0, 0);
    //     }
    //
    //     ATOMIC_SAFE_RELEASE(object_.ptr_);
    // }
    //
    // compilerOutput_.Clear();
    //
    // for (unsigned i = 0; i < MAX_TEXTURE_UNITS; ++i)
    //     useTextureUnit_[i] = false;
    // for (unsigned i = 0; i < MAX_SHADER_PARAMETER_GROUPS; ++i)
    //     constantBufferSizes_[i] = 0;
    // parameters_.Clear();
    // byteCode_.Clear();
    // elementHash_ = 0;
}

void ShaderVariation::SetDefines(const String& defines)
{
    throw std::exception("Not implemented");
    // defines_ = defines;
    //
    // // Internal mechanism for appending the CLIPPLANE define, prevents runtime (every frame) string manipulation
    // definesClipPlane_ = defines;
    // if (!definesClipPlane_.EndsWith(" CLIPPLANE"))
    //     definesClipPlane_ += " CLIPPLANE";
}

bool ShaderVariation::LoadByteCode(const String& binaryShaderName)
{
    throw std::exception("Not implemented");
    // ResourceCache* cache = owner_->GetSubsystem<ResourceCache>();
    // if (!cache->Exists(binaryShaderName))
    //     return false;
    //
    // FileSystem* fileSystem = owner_->GetSubsystem<FileSystem>();
    // unsigned sourceTimeStamp = owner_->GetTimeStamp();
    // // If source code is loaded from a package, its timestamp will be zero. Else check that binary is not older
    // // than source
    // if (sourceTimeStamp && fileSystem->GetLastModifiedTime(cache->GetResourceFileName(binaryShaderName)) < sourceTimeStamp)
    //     return false;
    //
    // SharedPtr<File> file = cache->GetFile(binaryShaderName);
    // if (!file || file->ReadFileID() != "USHD")
    // {
    //     ATOMIC_LOGERROR(binaryShaderName + " is not a valid shader bytecode file");
    //     return false;
    // }
    //
    // /// \todo Check that shader type and model match
    // /*unsigned short shaderType = */file->ReadUShort();
    // /*unsigned short shaderModel = */file->ReadUShort();
    // elementHash_ = file->ReadUInt();
    // elementHash_ <<= 32;
    //
    // unsigned numParameters = file->ReadUInt();
    // for (unsigned i = 0; i < numParameters; ++i)
    // {
    //     String name = file->ReadString();
    //     unsigned buffer = file->ReadUByte();
    //     unsigned offset = file->ReadUInt();
    //     unsigned size = file->ReadUInt();
    //
    //     ShaderParameter parameter;
    //     parameter.type_ = type_;
    //     parameter.name_ = name;
    //     parameter.buffer_ = buffer;
    //     parameter.offset_ = offset;
    //     parameter.size_ = size;
    //     parameters_[StringHash(name)] = parameter;
    // }
    //
    // unsigned numTextureUnits = file->ReadUInt();
    // for (unsigned i = 0; i < numTextureUnits; ++i)
    // {
    //     /*String unitName = */file->ReadString();
    //     unsigned reg = file->ReadUByte();
    //
    //     if (reg < MAX_TEXTURE_UNITS)
    //         useTextureUnit_[reg] = true;
    // }
    //
    // unsigned byteCodeSize = file->ReadUInt();
    // if (byteCodeSize)
    // {
    //     byteCode_.Resize(byteCodeSize);
    //     file->Read(&byteCode_[0], byteCodeSize);
    //
    //     if (type_ == VS)
    //         ATOMIC_LOGDEBUG("Loaded cached vertex shader " + GetFullName());
    //     else
    //         ATOMIC_LOGDEBUG("Loaded cached pixel shader " + GetFullName());
    //
    //     CalculateConstantBufferSizes();
    //     return true;
    // }
    // else
    // {
    //     ATOMIC_LOGERROR(binaryShaderName + " has zero length bytecode");
    //     return false;
    // }
}

bool ShaderVariation::Compile()
{
    throw std::exception("Not implemented");
//     const String& sourceCode = owner_->GetSourceCode(type_);
//     Vector<String> defines = defines_.Split(' ');
//
//     // Set the entrypoint, profile and flags according to the shader being compiled
//     const char* entryPoint = 0;
//     const char* profile = 0;
//     unsigned flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
//
//     defines.Push("D3D11");
//
//     if (type_ == VS)
//     {
//         entryPoint = "VS";
//         defines.Push("COMPILEVS");
//         profile = "vs_4_0";
//     }
//     else
//     {
//         entryPoint = "PS";
//         defines.Push("COMPILEPS");
//         profile = "ps_4_0";
//         flags |= D3DCOMPILE_PREFER_FLOW_CONTROL;
//     }
//
//     defines.Push("MAXBONES=" + String(Graphics::GetMaxBones()));
//
//     // Collect defines into macros
//     Vector<String> defineValues;
//     PODVector<D3D_SHADER_MACRO> macros;
//
//     for (unsigned i = 0; i < defines.Size(); ++i)
//     {
//         unsigned equalsPos = defines[i].Find('=');
//         if (equalsPos != String::NPOS)
//         {
//             defineValues.Push(defines[i].Substring(equalsPos + 1));
//             defines[i].Resize(equalsPos);
//         }
//         else
//             defineValues.Push("1");
//     }
//     for (unsigned i = 0; i < defines.Size(); ++i)
//     {
//         D3D_SHADER_MACRO macro;
//         macro.Name = defines[i].CString();
//         macro.Definition = defineValues[i].CString();
//         macros.Push(macro);
//
//         // In debug mode, check that all defines are referenced by the shader code
// #ifdef _DEBUG
//         if (sourceCode.Find(defines[i]) == String::NPOS)
//             ATOMIC_LOGWARNING("Shader " + GetFullName() + " does not use the define " + defines[i]);
// #endif
//     }
//
//     D3D_SHADER_MACRO endMacro;
//     endMacro.Name = 0;
//     endMacro.Definition = 0;
//     macros.Push(endMacro);
//
//     // Compile using D3DCompile
//     ID3DBlob* shaderCode = 0;
//     ID3DBlob* errorMsgs = 0;
//
//     HRESULT hr = D3DCompile(sourceCode.CString(), sourceCode.Length(), owner_->GetName().CString(), &macros.Front(), 0,
//         entryPoint, profile, flags, 0, &shaderCode, &errorMsgs);
//     if (FAILED(hr))
//     {
//         // Do not include end zero unnecessarily
//         compilerOutput_ = String((const char*)errorMsgs->GetBufferPointer(), (unsigned)errorMsgs->GetBufferSize() - 1);
//     }
//     else
//     {
//         if (type_ == VS)
//             ATOMIC_LOGDEBUG("Compiled vertex shader " + GetFullName());
//         else
//             ATOMIC_LOGDEBUG("Compiled pixel shader " + GetFullName());
//
//         unsigned char* bufData = (unsigned char*)shaderCode->GetBufferPointer();
//         unsigned bufSize = (unsigned)shaderCode->GetBufferSize();
//         // Use the original bytecode to reflect the parameters
//         ParseParameters(bufData, bufSize);
//         CalculateConstantBufferSizes();
//
//         // Then strip everything not necessary to use the shader
//         ID3DBlob* strippedCode = 0;
//         D3DStripShader(bufData, bufSize,
//             D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO | D3DCOMPILER_STRIP_TEST_BLOBS, &strippedCode);
//         byteCode_.Resize((unsigned)strippedCode->GetBufferSize());
//         memcpy(&byteCode_[0], strippedCode->GetBufferPointer(), byteCode_.Size());
//         strippedCode->Release();
//     }
//
//     ATOMIC_SAFE_RELEASE(shaderCode);
//     ATOMIC_SAFE_RELEASE(errorMsgs);
//     
//     return !byteCode_.Empty();
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
