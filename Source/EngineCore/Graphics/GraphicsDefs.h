//
// Copyright (c) 2008-2017 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "../Container/HashBase.h"
#include "../Math/StringHash.h"
#include "../Math/MathDefs.h"

#include <DiligentCore/Graphics/GraphicsEngine/interface/GraphicsTypes.h>

namespace Atomic
{

class Vector3;

/// Graphics capability support level. Web platform (Emscripten) also uses OpenGL ES, but is considered a desktop platform capability-wise
#if defined(IOS) || defined(TVOS) || defined(__ANDROID__) || defined(__arm__) || defined(__aarch64__)
#define MOBILE_GRAPHICS
#else
#define DESKTOP_GRAPHICS
#endif

typedef Diligent::TEXTURE_FORMAT TextureFormat;

enum class GraphicsBackend : uint8_t
{
    D3D11,
    D3D12,
    Vulkan,
    OpenGL,
    OpenGLES
};

enum GraphicsClearFlags
{
	GRAPHICS_CLEAR_SHADER_PROGRAMS      = 1 << 0,
    GRAPHICS_CLEAR_PIPELINES            = 1 << 2,
    GRAPHICS_CLEAR_SRB                  = 1 << 3,
    GRAPHICS_CLEAR_SCRATCH_BUFFERS      = 1 << 4,
    GRAPHICS_CLEAR_VERTEX_DECLARATIONS  = 1 << 5,
    GRAPHICS_CLEAR_CONSTANT_BUFFERS     = 1 << 6,
    GRAPHICS_CLEAR_TEXTURES             = 1 << 7,
    GRAPHICS_CLEAR_STG_TEXTURES         = 1 << 8,
    GRAPHICS_CLEAR_ALL = GRAPHICS_CLEAR_SHADER_PROGRAMS
	| GRAPHICS_CLEAR_PIPELINES
	| GRAPHICS_CLEAR_SRB
	| GRAPHICS_CLEAR_SCRATCH_BUFFERS
    | GRAPHICS_CLEAR_VERTEX_DECLARATIONS
    | GRAPHICS_CLEAR_CONSTANT_BUFFERS
    | GRAPHICS_CLEAR_TEXTURES
    | GRAPHICS_CLEAR_STG_TEXTURES
};
    
/// Primitive type.
enum PrimitiveType
{
    TRIANGLE_LIST = 0,
    LINE_LIST,
    POINT_LIST,
    TRIANGLE_STRIP,
    LINE_STRIP
};

/// %Geometry type for vertex shader geometry variations.
enum GeometryType
{
    GEOM_STATIC = 0,
    GEOM_SKINNED = 1,
    GEOM_INSTANCED = 2,
    GEOM_BILLBOARD = 3,
    GEOM_DIRBILLBOARD = 4,
    GEOM_TRAIL_FACE_CAMERA = 5,
    GEOM_TRAIL_BONE = 6,
    MAX_GEOMETRYTYPES = 7,
    // This is not a real geometry type for VS, but used to mark objects that do not desire to be instanced
    GEOM_STATIC_NOINSTANCING = 7,
};

/// Blending mode.
enum BlendMode
{
    BLEND_REPLACE = 0,
    BLEND_ADD,
    BLEND_MULTIPLY,
    BLEND_ALPHA,
    BLEND_ADDALPHA,
    BLEND_PREMULALPHA,
    BLEND_INVDESTALPHA,
    BLEND_SUBTRACT,
    BLEND_SUBTRACTALPHA,
    MAX_BLENDMODES
};

/// Depth or stencil compare mode.
enum CompareMode
{
    CMP_ALWAYS = 0,
    CMP_EQUAL,
    CMP_NOTEQUAL,
    CMP_LESS,
    CMP_LESSEQUAL,
    CMP_GREATER,
    CMP_GREATEREQUAL,
    MAX_COMPAREMODES
};

/// Culling mode.
enum CullMode
{
    CULL_NONE = 0,
    CULL_CCW,
    CULL_CW,
    MAX_CULLMODES
};

/// Fill mode.
enum FillMode
{
    FILL_SOLID = 0,
    FILL_WIREFRAME,
    FILL_POINT
};

/// Stencil operation.
enum StencilOp
{
    OP_KEEP = 0,
    OP_ZERO,
    OP_REF,
    OP_INCR,
    OP_DECR
};

/// Vertex/index buffer lock state.
enum LockState
{
    LOCK_NONE = 0,
    LOCK_HARDWARE,
    LOCK_SHADOW,
    LOCK_SCRATCH
};

/// Hardcoded legacy vertex elements.
enum LegacyVertexElement
{
    ELEMENT_POSITION = 0,
    ELEMENT_NORMAL,
    ELEMENT_COLOR,
    ELEMENT_TEXCOORD1,
    ELEMENT_TEXCOORD2,
    ELEMENT_CUBETEXCOORD1,
    ELEMENT_CUBETEXCOORD2,
    ELEMENT_TANGENT,
    ELEMENT_BLENDWEIGHTS,
    ELEMENT_BLENDINDICES,
    ELEMENT_INSTANCEMATRIX1,
    ELEMENT_INSTANCEMATRIX2,
    ELEMENT_INSTANCEMATRIX3,
    // Custom 32-bit integer object index. Due to API limitations, not supported on D3D9
    ELEMENT_OBJECTINDEX,
    MAX_LEGACY_VERTEX_ELEMENTS
};

/// Arbitrary vertex declaration element datatypes.
enum VertexElementType : uint8_t
{
    TYPE_INT = 0,
    TYPE_FLOAT,
    TYPE_VECTOR2,
    TYPE_VECTOR3,
    TYPE_VECTOR4,
    TYPE_UBYTE4,
    TYPE_UBYTE4_NORM,
    MAX_VERTEX_ELEMENT_TYPES
};

/// Arbitrary vertex declaration element semantics.
enum VertexElementSemantic
{
    SEM_POSITION = 0,
    SEM_NORMAL,
    SEM_BINORMAL,
    SEM_TANGENT,
    SEM_TEXCOORD,
    SEM_COLOR,
    SEM_BLENDWEIGHTS,
    SEM_BLENDINDICES,
    SEM_OBJECTINDEX,
    MAX_VERTEX_ELEMENT_SEMANTICS
};

/// Vertex element description for arbitrary vertex declarations.
struct ATOMIC_API VertexElement
{
    /// Default-construct.
    VertexElement() :
        type_(TYPE_VECTOR3),
        semantic_(SEM_POSITION),
        index_(0),
        perInstance_(false),
        offset_(0)
    {
    }

    /// Construct with type, semantic, index and whether is per-instance data.
    VertexElement(VertexElementType type, VertexElementSemantic semantic, unsigned char index = 0, bool perInstance = false) :
        type_(type),
        semantic_(semantic),
        index_(index),
        perInstance_(perInstance),
        offset_(0)
    {
    }

    /// Test for equality with another vertex element. Offset is intentionally not compared, as it's relevant only when an element exists within a vertex buffer.
    bool operator ==(const VertexElement& rhs) const { return type_ == rhs.type_ && semantic_ == rhs.semantic_ && index_ == rhs.index_ && perInstance_ == rhs.perInstance_; }

    /// Test for inequality with another vertex element.
    bool operator !=(const VertexElement& rhs) const { return !(*this == rhs); }

    /// Data type of element.
    VertexElementType type_;
    /// Semantic of element.
    VertexElementSemantic semantic_;
    /// Semantic index of element, for example multi-texcoords.
    unsigned char index_;
    /// Per-instance flag.
    bool perInstance_;
    /// Offset of element from vertex start. Filled by VertexBuffer once the vertex declaration is built.
    unsigned offset_;
};

/// Sizes of vertex element types.
extern ATOMIC_API const unsigned ELEMENT_TYPESIZES[];

/// Vertex element definitions for the legacy elements.
extern ATOMIC_API const VertexElement LEGACY_VERTEXELEMENTS[];

/// Texture filtering mode.
enum TextureFilterMode
{
    FILTER_NEAREST = 0,
    FILTER_BILINEAR,
    FILTER_TRILINEAR,
    FILTER_ANISOTROPIC,
    FILTER_NEAREST_ANISOTROPIC,
    FILTER_DEFAULT,
    MAX_FILTERMODES
};

/// Texture addressing mode.
enum TextureAddressMode
{
    ADDRESS_WRAP = 0,
    ADDRESS_MIRROR,
    ADDRESS_CLAMP,
    ADDRESS_BORDER,
    MAX_ADDRESSMODES
};

/// Texture coordinates.
enum TextureCoordinate
{
    COORD_U = 0,
    COORD_V,
    COORD_W,
    MAX_COORDS
};

/// Texture usage types.
enum TextureUsage
{
    TEXTURE_STATIC = 0,
    TEXTURE_DYNAMIC,
    TEXTURE_RENDERTARGET,
    TEXTURE_DEPTHSTENCIL
};

/// Cube map faces.
enum CubeMapFace
{
    FACE_POSITIVE_X = 0,
    FACE_NEGATIVE_X,
    FACE_POSITIVE_Y,
    FACE_NEGATIVE_Y,
    FACE_POSITIVE_Z,
    FACE_NEGATIVE_Z,
    MAX_CUBEMAP_FACES
};

/// Cubemap single image layout modes.
enum CubeMapLayout
{
    CML_HORIZONTAL = 0,
    CML_HORIZONTALNVIDIA,
    CML_HORIZONTALCROSS,
    CML_VERTICALCROSS,
    CML_BLENDER
};

/// Update mode for render surface viewports.
enum RenderSurfaceUpdateMode
{
    SURFACE_MANUALUPDATE = 0,
    SURFACE_UPDATEVISIBLE,
    SURFACE_UPDATEALWAYS
};

/// Shader types.
enum ShaderType : uint8_t
{
    VS = 0,
    PS,
    MAX_SHADER_TYPES
};

/// Shader parameter groups for determining need to update. On APIs that support constant buffers, these correspond to different constant buffers.
enum ShaderParameterGroup
{
    SP_FRAME = 0,
    SP_CAMERA,
    SP_ZONE,
    SP_LIGHT,
    SP_MATERIAL,
    SP_OBJECT,
    SP_CUSTOM,
    MAX_SHADER_PARAMETER_GROUPS
};

/// Texture units.
enum TextureUnit
{
    TU_DIFFUSE = 0,
    TU_NORMAL,
    TU_SPECULAR,
    TU_EMISSIVE,
    TU_ENVIRONMENT,
    TU_VOLUMEMAP,
    TU_CUSTOM1,
    TU_CUSTOM2,
    TU_LIGHTRAMP,
    TU_LIGHTSHAPE,
    TU_SHADOWMAP,
    TU_FACESELECT,
    TU_INDIRECTION,
    TU_DEPTHBUFFER,
    TU_LIGHTBUFFER,
    TU_ZONE,
    TU_ALBEDOBUFFER,
    TU_NORMALBUFFER,
    MAX_TEXTURE_UNITS,
};

#define MAX_MATERIAL_TEXTURE_UNITS 8

enum class TextureUnitType
{
    Undefined = 0,
	Texture2D,
    Texture3D,
    TextureCube
};

/// Billboard camera facing modes.
enum FaceCameraMode
{
    FC_NONE = 0,
    FC_ROTATE_XYZ,
    FC_ROTATE_Y,
    FC_LOOKAT_XYZ,
    FC_LOOKAT_Y,
    FC_LOOKAT_MIXED,
    FC_DIRECTION,
};

/// Shadow type.
enum ShadowQuality
{
    SHADOWQUALITY_SIMPLE_16BIT = 0,
    SHADOWQUALITY_SIMPLE_24BIT,
    SHADOWQUALITY_PCF_16BIT,
    SHADOWQUALITY_PCF_24BIT,
    SHADOWQUALITY_VSM,
    SHADOWQUALITY_BLUR_VSM
};

/// %Shader parameter definition.
struct ATOMIC_API ShaderParameter
{
    /// %Shader type.
    ShaderType type_{MAX_SHADER_TYPES};
    /// Name of the parameter.
    String name_{};

    union
    {
        /// Offset in constant buffer.
        unsigned offset_;
        /// OpenGL uniform location.
        int location_;
        /// Direct3D9 register index.
        unsigned register_;
    };

    union
    {
        /// Parameter size. Used only on Direct3D11 to calculate constant buffer size.
        unsigned size_;
        /// Parameter OpenGL type.
        unsigned glType_;
        /// Number of registers on Direct3D9.
        unsigned regCount_;
    };

    /// Constant buffer index. Only used on Direct3D11.
    unsigned buffer_{M_MAX_UNSIGNED};
    /// Constant buffer pointer. Defined only in shader programs.
    void* bufferPtr_{nullptr};
};

enum class ShaderByteCodeType : uint8_t
{
    Raw,
	SpirV,
    DxB,
    Max
};

// Inbuilt shader parameters.
extern ATOMIC_API const StringHash VSP_AMBIENTSTARTCOLOR;
extern ATOMIC_API const StringHash VSP_AMBIENTENDCOLOR;
extern ATOMIC_API const StringHash VSP_BILLBOARDROT;
extern ATOMIC_API const StringHash VSP_CAMERAPOS;
extern ATOMIC_API const StringHash VSP_CLIPPLANE;
extern ATOMIC_API const StringHash VSP_NEARCLIP;
extern ATOMIC_API const StringHash VSP_FARCLIP;
extern ATOMIC_API const StringHash VSP_DEPTHMODE;
extern ATOMIC_API const StringHash VSP_DELTATIME;
extern ATOMIC_API const StringHash VSP_ELAPSEDTIME;
extern ATOMIC_API const StringHash VSP_FRUSTUMSIZE;
extern ATOMIC_API const StringHash VSP_GBUFFEROFFSETS;
extern ATOMIC_API const StringHash VSP_LIGHTDIR;
extern ATOMIC_API const StringHash VSP_LIGHTPOS;
extern ATOMIC_API const StringHash VSP_NORMALOFFSETSCALE;
extern ATOMIC_API const StringHash VSP_MODEL;
extern ATOMIC_API const StringHash VSP_VIEW;
extern ATOMIC_API const StringHash VSP_VIEWINV;
extern ATOMIC_API const StringHash VSP_VIEWPROJ;
extern ATOMIC_API const StringHash VSP_UOFFSET;
extern ATOMIC_API const StringHash VSP_VOFFSET;
extern ATOMIC_API const StringHash VSP_ZONE;
extern ATOMIC_API const StringHash VSP_LIGHTMATRICES;
extern ATOMIC_API const StringHash VSP_SKINMATRICES;
extern ATOMIC_API const StringHash VSP_VERTEXLIGHTS;
extern ATOMIC_API const StringHash PSP_AMBIENTCOLOR;
extern ATOMIC_API const StringHash PSP_CAMERAPOS;
extern ATOMIC_API const StringHash PSP_DELTATIME;
extern ATOMIC_API const StringHash PSP_DEPTHRECONSTRUCT;
extern ATOMIC_API const StringHash PSP_ELAPSEDTIME;
extern ATOMIC_API const StringHash PSP_FOGCOLOR;
extern ATOMIC_API const StringHash PSP_FOGPARAMS;
extern ATOMIC_API const StringHash PSP_GBUFFERINVSIZE;
extern ATOMIC_API const StringHash PSP_LIGHTCOLOR;
extern ATOMIC_API const StringHash PSP_LIGHTDIR;
extern ATOMIC_API const StringHash PSP_LIGHTPOS;
extern ATOMIC_API const StringHash PSP_NORMALOFFSETSCALE;
extern ATOMIC_API const StringHash PSP_MATDIFFCOLOR;
extern ATOMIC_API const StringHash PSP_MATEMISSIVECOLOR;
extern ATOMIC_API const StringHash PSP_MATENVMAPCOLOR;
extern ATOMIC_API const StringHash PSP_MATSPECCOLOR;
extern ATOMIC_API const StringHash PSP_NEARCLIP;
extern ATOMIC_API const StringHash PSP_FARCLIP;
extern ATOMIC_API const StringHash PSP_SHADOWCUBEADJUST;
extern ATOMIC_API const StringHash PSP_SHADOWDEPTHFADE;
extern ATOMIC_API const StringHash PSP_SHADOWINTENSITY;
extern ATOMIC_API const StringHash PSP_SHADOWMAPINVSIZE;
extern ATOMIC_API const StringHash PSP_SHADOWSPLITS;
extern ATOMIC_API const StringHash PSP_LIGHTMATRICES;
extern ATOMIC_API const StringHash PSP_VSMSHADOWPARAMS;
extern ATOMIC_API const StringHash PSP_ROUGHNESS;
extern ATOMIC_API const StringHash PSP_METALLIC;
extern ATOMIC_API const StringHash PSP_LIGHTRAD;
extern ATOMIC_API const StringHash PSP_LIGHTLENGTH;
extern ATOMIC_API const StringHash PSP_ZONEMIN;
extern ATOMIC_API const StringHash PSP_ZONEMAX;

// Scale calculation from bounding box diagonal.
extern ATOMIC_API const Vector3 DOT_SCALE;

static const int QUALITY_LOW = 0;
static const int QUALITY_MEDIUM = 1;
static const int QUALITY_HIGH = 2;
static const int QUALITY_MAX = 15;

static const unsigned CLEAR_COLOR = 0x1;
static const unsigned CLEAR_DEPTH = 0x2;
static const unsigned CLEAR_STENCIL = 0x4;

// Legacy vertex element bitmasks.
static const unsigned MASK_NONE = 0x0;
static const unsigned MASK_POSITION = 0x1;
static const unsigned MASK_NORMAL = 0x2;
static const unsigned MASK_COLOR = 0x4;
static const unsigned MASK_TEXCOORD1 = 0x8;
static const unsigned MASK_TEXCOORD2 = 0x10;
static const unsigned MASK_CUBETEXCOORD1 = 0x20;
static const unsigned MASK_CUBETEXCOORD2 = 0x40;
static const unsigned MASK_TANGENT = 0x80;
static const unsigned MASK_BLENDWEIGHTS = 0x100;
static const unsigned MASK_BLENDINDICES = 0x200;
static const unsigned MASK_INSTANCEMATRIX1 = 0x400;
static const unsigned MASK_INSTANCEMATRIX2 = 0x800;
static const unsigned MASK_INSTANCEMATRIX3 = 0x1000;
static const unsigned MASK_OBJECTINDEX = 0x2000;

static const u8 MAX_RENDERTARGETS = 4;
static const int MAX_VERTEX_STREAMS = 4;
static const int MAX_CONSTANT_REGISTERS = 256;
static const int MAX_IMMUTABLE_SAMPLERS = 16;
    
static const int BITS_PER_COMPONENT = 8;
}
