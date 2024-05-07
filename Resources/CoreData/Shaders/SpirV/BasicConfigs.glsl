#ifndef BASIC_CONFIGS_GLSL
#define BASIC_CONFIGS_GLSL 1

// #extension GL_EXT_clip_cull_distance: enable
#extension GL_OES_standard_derivatives : enable

#ifdef GL_ES
    precision mediump float;
#endif

#define highp
#define mediump
#define lowp

#define half   mediump float
#define half2  mediump vec2
#define half3  mediump vec3
#define half4  mediump vec4
#define fixed  lowp float
#define fixed2 lowp vec2
#define fixed3 lowp vec3
#define fixed4 lowp vec4

#endif