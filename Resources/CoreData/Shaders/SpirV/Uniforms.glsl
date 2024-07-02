#include "BasicConfigs.glsl"

#if IS_GLES && !RENGINE_PLATFORM_WINDOWS
    #define UNIFORM_BEGIN(name) uniform name {
#else
    #define UNIFORM_BEGIN(name) layout(std140) uniform name {
#endif

#define UNIFORM_END() };

#ifdef COMPILEVS
    UNIFORM_BEGIN(FrameVS)
        float cDeltaTime;
        float cElapsedTime;
    UNIFORM_END()
    
    UNIFORM_BEGIN(CameraVS)
        vec3 cCameraPos;
        float cNearClip;
        float cFarClip;
        vec4 cDepthMode;
        vec3 cFrustumSize;
        vec4 cGBufferOffsets;
        mat4 cView;
        mat4 cViewInv;
        mat4 cViewProj;
        vec4 cClipPlane;
    UNIFORM_END()
    
    UNIFORM_BEGIN(ZoneVS)
        vec3 cAmbientStartColor;
        vec3 cAmbientEndColor;
        mat4 cZone;
    UNIFORM_END()
    
    UNIFORM_BEGIN(LightVS)
        vec4 cLightPos;
        vec3 cLightDir;
        vec4 cNormalOffsetScale;
        #ifdef NUMVERTEXLIGHTS
            vec4 cVertexLights[4 * 3];
        #else
            mat4 cLightMatrices[4];
        #endif // NUMVERTEXLIGHTS
    UNIFORM_END()
    
    #ifndef CUSTOM_MATERIAL_CBUFFER
        UNIFORM_BEGIN(MaterialVS)
            vec4 cUOffset;
            vec4 cVOffset;
        UNIFORM_END()
    #endif // CUSTOM_MATERIAL_CBUFFER
    
    UNIFORM_BEGIN(ObjectVS)
        mat4 cModel;
        mat3 cBillboardRot;
        vec4 cSkinMatrices[MAXBONES*3];
    UNIFORM_END()

#endif // COMPILEVS

#ifdef COMPILEPS
    // Pixel shader uniforms
    UNIFORM_BEGIN(FramePS)
        float cDeltaTimePS;
        float cElapsedTimePS;
    UNIFORM_END()
    
    UNIFORM_BEGIN(CameraPS)
        vec3 cCameraPosPS;
        vec4 cDepthReconstruct;
        vec2 cGBufferInvSize;
        float cNearClipPS;
        float cFarClipPS;
    UNIFORM_END()
    
    UNIFORM_BEGIN(ZonePS)
        vec4 cAmbientColor;
        vec4 cFogParams;
        vec3 cFogColor;
        vec3 cZoneMin;
        vec3 cZoneMax;
    UNIFORM_END()
    
    UNIFORM_BEGIN(LightPS)
        vec4 cLightColor;
        vec4 cLightPosPS;
        vec3 cLightDirPS;
        vec4 cNormalOffsetScalePS;
        vec4 cShadowCubeAdjust;
        vec4 cShadowDepthFade;
        vec2 cShadowIntensity;
        vec2 cShadowMapInvSize;
        vec4 cShadowSplits;
        mat4 cLightMatricesPS[4];
        #ifdef VSM_SHADOW
            vec2 cVSMShadowParams;
        #endif
        #ifdef PBR
            float cLightRad;
            float cLightLength;
        #endif
    UNIFORM_END()
    
    #ifndef CUSTOM_MATERIAL_CBUFFER
        UNIFORM_BEGIN(MaterialPS)
            vec4 cMatDiffColor;
            vec3 cMatEmissiveColor;
            vec3 cMatEnvMapColor;
            vec4 cMatSpecColor;
            #ifdef PBR
                float cRoughness;
                float cMetallic;
            #endif
        UNIFORM_END()
    #endif
#endif // COMPILEPS
