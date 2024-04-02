#ifdef COMPILEVS
    layout(binding=0) uniform FrameVS
    {
        float cDeltaTime;
        float cElapsedTime;
    };
    
    layout(binding=1) uniform CameraVS
    {
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
    };
    
    layout(binding=2) uniform ZoneVS
    {
        vec3 cAmbientStartColor;
        vec3 cAmbientEndColor;
        mat4 cZone;
    };
    
    layout(binding=3) uniform LightVS
    {
        vec4 cLightPos;
        vec3 cLightDir;
        vec4 cNormalOffsetScale;
        #ifdef NUMVERTEXLIGHTS
            vec4 cVertexLights[4 * 3];
        #else
            mat4 cLightMatrices[4];
        #endif // NUMVERTEXLIGHTS
    };
    
    #ifndef CUSTOM_MATERIAL_CBUFFER
        layout(binding=5) uniform MaterialVS
        {
            vec4 cUOffset;
            vec4 cVOffset;
        };
    #endif // CUSTOM_MATERIAL_CBUFFER
    
    layout(std140) uniform ObjectVS
    {
        mat4 cModel;
        #ifdef BILLBOARD
            mat3 cBillboardRot;
        #endif // BILLBOARD
        #ifdef SKINNED
            vec4 cSkinMatrices[MAXBONES*3];
        #endif // SKINNED
    };

#endif // COMPILEVS

#ifdef COMPILEPS
    // Pixel shader uniforms
    layout(binding=0) uniform FramePS
    {
        float cDeltaTimePS;
        float cElapsedTimePS;
    };
    
    layout(binding=1) uniform CameraPS
    {
        vec3 cCameraPosPS;
        vec4 cDepthReconstruct;
        vec2 cGBufferInvSize;
        float cNearClipPS;
        float cFarClipPS;
    };
    
    layout(binding=2) uniform ZonePS
    {
        vec4 cAmbientColor;
        vec4 cFogParams;
        vec3 cFogColor;
        vec3 cZoneMin;
        vec3 cZoneMax;
    };
    
    layout(binding=3) uniform LightPS
    {
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
    };
    
    #ifndef CUSTOM_MATERIAL_CBUFFER
        layout(binding=4) uniform MaterialPS
        {
            vec4 cMatDiffColor;
            vec3 cMatEmissiveColor;
            vec3 cMatEnvMapColor;
            vec4 cMatSpecColor;
            #ifdef PBR
                float cRoughness;
                float cMetallic;
            #endif
        };
    #endif
#endif // COMPILEPS
