#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "Lighting.glsl"

varying vec3 vTexCoord;

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    gl_Position.z = gl_Position.w;
    vTexCoord = iPos.xyz;
}

void PS()
{
#ifdef LIGHTSCATTERINGMASK
    vec4 skyColor = cMatDiffColor * textureCube(sDiffCubeMap, vTexCoord);
    gl_FragData[0] = skyColor;
    
    //if (GetIntensity(skyColor.rgb) > 0.8f)
    //    gl_FragData[1] = vec4(1.0);
    //else
    //    gl_FragData[1] = vec4(0.0);

    float t = step(GetIntensity(skyColor.rgb), 0.8);
    gl_FragData[1] = mix(vec4(1.0), vec4(0.0), t);
    
    //TODO: ? this mask can be written into Alpha channel of viewport rtt and exctact from it until blend op passes
#else
    gl_FragColor = cMatDiffColor * textureCube(sDiffCubeMap, vTexCoord);
#endif
}
