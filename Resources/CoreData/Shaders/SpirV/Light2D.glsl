#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"

varying vec4 vColor;

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vColor = iColor * iTexCoord.x;
}

void PS()
{
	gl_FragColor.rgba = vColor;
}

