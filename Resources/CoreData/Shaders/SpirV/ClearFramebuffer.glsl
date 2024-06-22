#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "ScreenPos.glsl"

#if defined(COMPILEPS)
    out vec4 fragData[1];
    #define gl_FragColor fragData[0]
#endif

void VS()
{
    mat4 modelMatrix = cModel;
    vec4 pos_vertices[4];
    pos_vertices[0] = vec4(-1.0, -1.0, 0.0, 1.0);
    pos_vertices[1] = vec4(-1.0, +1.0, 0.0, 1.0);
    pos_vertices[2] = vec4(+1.0, -1.0, 0.0, 1.0);
    pos_vertices[3] = vec4(+1.0, +1.0, 0.0, 1.0);

    vec3 worldPos = (pos_vertices[gl_VertexID] * modelMatrix).xyz;
    gl_Position = vec4(worldPos, 1.0) * cViewProj;
}

void PS()
{
    gl_FragColor = cMatDiffColor;
}

