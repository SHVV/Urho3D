#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"

varying vec2 vTexCoord;
varying vec2 vScreenPos;

#ifdef COMPILEPS
uniform float cNumSamples;
uniform float cExposure;
uniform float cDecay;
uniform float cDensity;
uniform float cWeight;
uniform vec2 cLightPositionOnScreen;
uniform vec2 csuncircleInvSize;

// TODO: for optimize group all single floats into vec4 (vec4 into mat4) and excract single values with Define for less glUniform... calling
// uniform vec4 cParams;
// #define cNumSamples cParams[0]
// #define cExposure cParams[1]
// and so on..

#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vTexCoord = GetQuadTexCoord(gl_Position);
    vScreenPos = GetScreenPosPreDiv(gl_Position);
}

void PS()
{
    vec2 deltaTexCoords = vec2(vScreenPos - cLightPositionOnScreen);
    deltaTexCoords *= 1.0 / float(cNumSamples) * cDensity;
    float illyminationDecay = 1.0;
    vec2 texCoords = vScreenPos;
    vec4 color = vec4(0.0);
    for (int i = 0; i < cNumSamples; i++) 
    {
            texCoords -= deltaTexCoords;
            vec4 sample = texture2D(sDiffMap, texCoords); 
            sample *= illyminationDecay * cWeight;
            color += sample;
            illyminationDecay *= cDecay;    
    }
    
     gl_FragColor = vec4(vec3(color * cExposure), 1.0);
}

