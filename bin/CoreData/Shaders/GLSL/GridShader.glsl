#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "Fog.glsl"

varying vec2 vTexCoord;
varying vec4 vWorldPos;
varying vec3 vViewPos;
#ifdef VERTEXCOLOR
    varying vec4 vColor;
#endif

uniform float cUse2DGrid;

#ifdef COMPILEVS

vec3 GetViewPos(mat4 modelMatrix) 
{
    return ((iPos * modelMatrix) * cView).xyz;
}

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    vec3 viewPos = GetViewPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vTexCoord = GetTexCoord(iTexCoord);
    vWorldPos = vec4(worldPos, GetDepth(gl_Position));
    vViewPos = viewPos;

    #ifdef VERTEXCOLOR
        vColor = iColor;
    #endif
}
#endif

void PS()
{
    float howFar = 1.0 / length(vViewPos.xyz);
    //float howFar = 1.0 / vViewPos.z;
    
    // Get material diffuse albedo
    #ifdef DIFFMAP
        vec4 diffColor = cMatDiffColor * texture2D(sDiffMap, vTexCoord);
        #ifdef ALPHAMASK
            if (diffColor.a < 0.5)
                discard;
        #endif
    #else
        vec4 diffColor = cMatDiffColor;
    #endif

    #ifdef VERTEXCOLOR
        diffColor *= vColor;
    #endif
    
    vec3 planeNormalUp = vec3(0.0, 1.0, 0.0);
    vec3 planeNormalDown = vec3(0.0, -1.0, 0.0);
    vec3 planeNormalForward  = vec3(0.0,0.0, 1.0);
    vec3 planeNormalBack  = vec3(0.0,0.0, -1.0);
    
    vec3 eyeNormalToGrid = normalize(cCameraPosPS - vWorldPos.xyz);
    vec3 planeNormalA = mix(planeNormalUp, planeNormalForward, cUse2DGrid);
    vec3 planeNormalB = mix(planeNormalDown, planeNormalBack, cUse2DGrid);
    
    float dotWithGridSideA = mix(-0.5, 32.0, clamp(dot(planeNormalA, eyeNormalToGrid), 0, 1.0));
    float dotWithGridSideB = mix(-0.5, 32.0, clamp(dot(planeNormalB, eyeNormalToGrid), 0, 1.0));
    
    diffColor.a = howFar * (dotWithGridSideA + dotWithGridSideB);

    // Get fog factor
    #ifdef HEIGHTFOG
        float fogFactor = GetHeightFogFactor(vWorldPos.w, vWorldPos.y);
    #else
        float fogFactor = GetFogFactor(vWorldPos.w);
    #endif

    #if defined(PREPASS)
        // Fill light pre-pass G-Buffer
        gl_FragData[0] = vec4(0.5, 0.5, 0.5, 1.0);
        gl_FragData[1] = vec4(EncodeDepth(vWorldPos.w), 0.0);
    #elif defined(DEFERRED)
        gl_FragData[0] = vec4(GetFog(diffColor.rgb, fogFactor), diffColor.a);
        gl_FragData[1] = vec4(0.0, 0.0, 0.0, 0.0);
        gl_FragData[2] = vec4(0.5, 0.5, 0.5, 1.0);
        gl_FragData[3] = vec4(EncodeDepth(vWorldPos.w), 0.0);
    #else
        gl_FragColor = vec4(GetFog(diffColor.rgb, fogFactor), diffColor.a);
    #endif
}
