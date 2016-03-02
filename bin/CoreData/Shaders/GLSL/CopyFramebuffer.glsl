#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"

varying vec2 vScreenPos;

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vScreenPos = GetScreenPosPreDiv(gl_Position);
}

void PS()
{
#ifdef REDTORGB
    gl_FragColor.rgb = texture2D(sDiffMap, vScreenPos).rrr;
#else
    gl_FragColor = texture2D(sDiffMap, vScreenPos);
#endif
}

