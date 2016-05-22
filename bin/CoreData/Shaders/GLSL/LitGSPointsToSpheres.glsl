#ifndef COMPILEGS //ONLY IF NOT GS Shader

#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "Fog.glsl"
#include "Lighting.glsl"

#endif //ONLY IF NOT GS Shader

uniform float cRadius;

//---------------------------------------------VS-----------------------------------------------------
#ifdef COMPILEVS

#ifdef VERTEXCOLOR
out vec4 vColor;
#endif
varying vec3 vVertexLight;

void VS()
{
  mat4 modelMatrix = iModelMatrix;
  vec4 WorldPos = iPos * modelMatrix;
  gl_Position = WorldPos * cView;
  vVertexLight = GetAmbient(GetZonePos(WorldPos.xyz));
  //mat4 modelMatrix = iModelMatrix;
  //gl_Position = vec4(GetWorldPos(modelMatrix), 1.0);
  
  //mat4 MV = cModel * cView;
  //gl_Position = iPos * MV;

#ifdef VERTEXCOLOR
  vColor = iColor;
#endif
}
#endif // COMPILEVS

//---------------------------------------------GS-----------------------------------------------------
#ifdef COMPILEGS

#include "Uniforms.glsl"
#include "Transform.glsl"

uniform mat4 cModel;
uniform mat4 cViewProj;
uniform mat4 cViewInv;
uniform vec4 cDepthMode;
uniform vec3 cCameraPos;
uniform vec3 cCameraRot;
uniform mat4 cProj;

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

#ifdef VERTEXCOLOR
  in vec4 vColor[1];
  out vec4 gColor;
#endif

out vec4 gWorldPos;

in vec3 vVertexLight[1];
out vec3 gVertexLight;

mat3 GetNormalMatrix(mat4 modelMatrix)
{
  return mat3(modelMatrix[0].xyz, modelMatrix[1].xyz, modelMatrix[2].xyz);
}

float GetDepth(vec4 clipPos)
{
  return dot(clipPos.zw, cDepthMode.zw);
}

void GS()
{
  vec3 quad[4];

  bool ortographic = cDepthMode.x == 1.0;
  vec4 iPos = gl_in[0].gl_Position;
  vec3 toCamera = ortographic ? vec3(0.0, 0.0, -1.0) : - normalize(iPos.xyz);
  vec3 up = vec3(0.0, 1.0, 0.0);
  vec3 right = ortographic ? vec3(1.0, 0.0, 0.0) : normalize(cross(toCamera, up));
  if (!ortographic) {
    up = normalize(cross(right, toCamera));
  }

  float size = cRadius * 1;

  toCamera *= size;
  quad[0] = iPos.xyz - (right + up) * size;
  quad[1] = iPos.xyz - (right - up) * size;
  quad[2] = iPos.xyz + (right - up) * size;
  quad[3] = iPos.xyz + (right + up) * size;

  for (int i = 0; i < 4; i++) {
    vec4 quadVertexWp = vec4(quad[i].xyz + toCamera, iPos.w);
    gl_Position = quadVertexWp * cProj;
    gWorldPos = vec4(iPos.xyz, GetDepth(gl_Position));
#ifdef VERTEXCOLOR
    gColor = vColor[0];
#endif
    vec4 WorldPos = vec4(iPos.xyz, 1.0) * cViewInv;
    gVertexLight = vVertexLight[0];
    EmitVertex();
  }
  EndPrimitive();
}
#endif // COMPILEGS

//---------------------------------------------PS-----------------------------------------------------
#ifdef COMPILEPS

in vec4 gWorldPos;

#ifdef VERTEXCOLOR
varying vec4 gColor;
#endif
varying vec3 gVertexLight;

uniform vec3 cFrustumSize;
uniform mat4 cProj;
uniform mat4 cViewInv;
uniform vec4 cDepthMode;
uniform mat4 cLightMatrices[4];
uniform vec4 cLightPos;
uniform vec2 cGBufferOffset;

#extension GL_ARB_conservative_depth : enable
layout(depth_greater) out float gl_FragDepth;
layout(origin_upper_left, pixel_center_integer) in vec4 gl_FragCoord;

float GetDepth(vec4 clipPos)
{
  return dot(clipPos.zw, cDepthMode.zw);
}

mat3 GetNormalMatrix(mat4 modelMatrix)
{
  return mat3(modelMatrix[0].xyz, modelMatrix[1].xyz, modelMatrix[2].xyz);
}

#ifdef SHADOW

#if defined(DIRLIGHT) && (!defined(GL_ES) || defined(WEBGL))
#define NUMCASCADES 4
#else
#define NUMCASCADES 1
#endif

vec4 GetShadowPos(int index, vec4 projWorldPos)
{
#if defined(DIRLIGHT)
  return projWorldPos * cLightMatrices[index];
#elif defined(SPOTLIGHT)
  return projWorldPos * cLightMatrices[1];
#else
  return vec4(projWorldPos.xyz - cLightPos.xyz, 1.0);
#endif
}

#endif

void PS()
{
#ifdef SHADOW_MAP
  return;
#endif
  vec3 ray_vec = cDepthReconstruct.x == 1.0 ?
    vec3(0, 0, 1) :
    normalize(vec3(((gl_FragCoord.xy - cGBufferOffset) * cGBufferInvSize.xy - vec2(0.5, 0.5)) * 2 * cFrustumSize.xy, cFrustumSize.z));
  ray_vec.y = -ray_vec.y;
  vec3 origin = cDepthReconstruct.x == 1.0 ?
    vec3(((gl_FragCoord.xy - cGBufferOffset) * cGBufferInvSize.xy - vec2(0.5, 0.5)) * 2 * cFrustumSize.xy, 0) :
    vec3(0, 0, 0);
  origin.y = -origin.y;
  //vec3 ray_vec = vec3((gl_FragCoord.xy - cGBufferOffset) * cGBufferInvSize.xy, 0);
  //vec3 norm_pos = gWorldPos.xyz - origin;
  //gl_FragColor = vec4(ray_vec.xyz, 1.0);
  //return;
  float a = 1;
  float b = -2 * dot(ray_vec, gWorldPos.xyz - origin);
  float c = dot(gWorldPos.xyz - origin, gWorldPos.xyz - origin) - cRadius * cRadius;
  float D = b * b - 4 * a * c;
  if (D < 0) {
    discard;
  }
  float t = (-b - sqrt(D)) / (2 * a);
  vec3 pos = ray_vec * t + origin;
  vec3 normal = normalize(pos - gWorldPos.xyz);
  vec4 proj_pos = vec4(pos, 1.0) * cProj;
  vec4 WorldPos = vec4((vec4(pos, 1.0) * cViewInv).xyz, GetDepth(proj_pos));
  gl_FragDepth = 0.5 * (proj_pos.z / proj_pos.w) + 0.5;

#ifdef SHADOW_MAP
  #ifdef VSM_SHADOW
    float depth = gl_FragDepth * 0.5 + 0.5;
    float moment1 = depth;
    float moment2 = depth * depth;

    // Adjusting moments (this is sort of bias per pixel) using partial derivative
    float dx = dFdx(depth);
    float dy = dFdy(depth);
    moment2 += 0.25*(dx*dx + dy*dy);

    gl_FragColor = vec4(moment1, moment2, 0.0, 0.0);
  #else
    gl_FragColor = vec4(1.0);
  #endif
    return;
#endif
    normal = normal * GetNormalMatrix(cViewInv);
    //gl_FragColor = vec4(WorldPos.xyz, 1.0);
    //gl_FragColor = vec4(normal.xyz, 1.0);

    // Get material diffuse albedo
    // TODO: add support for diffuse texture 
    /*#ifdef DIFFMAP
      vec4 diffInput = texture2D(sDiffMap, vTexCoord.xy);
      #ifdef ALPHAMASK
        if (diffInput.a < 0.5)
          discard;
      #endif
      vec4 diffColor = cMatDiffColor * diffInput;
    #else*/
      vec4 diffColor = cMatDiffColor;
    //#endif

    #ifdef VERTEXCOLOR
      diffColor *= gColor;
    #endif
    
    // Get material specular albedo
    // TODO: Add support for specular map
    /*#ifdef SPECMAP
      vec3 specColor = cMatSpecColor.rgb * texture2D(sSpecMap, vTexCoord.xy).rgb;
    #else*/
      vec3 specColor = cMatSpecColor.rgb;
    //#endif

    // Get normal
    // TODO: Add support for normal map
    /*#ifdef NORMALMAP
      mat3 tbn = mat3(vTangent.xyz, vec3(vTexCoord.zw, vTangent.w), vNormal);
      vec3 normal = normalize(tbn * DecodeNormal(texture2D(sNormalMap, vTexCoord.xy)));
    #else
      vec3 normal = normalize(vNormal);
    #endif*/

    // Get fog factor
    #ifdef HEIGHTFOG
        float fogFactor = GetHeightFogFactor(WorldPos.w, WorldPos.y);
    #else
        float fogFactor = GetFogFactor(WorldPos.w);
    #endif

    #if defined(PERPIXEL)
        // Per-pixel forward lighting
        vec3 lightColor;
        vec3 lightDir;
        vec3 finalColor;

        float diff = GetDiffuse(normal, WorldPos.xyz, lightDir);

        #ifdef SHADOW
          vec4 projWorldPos = vec4(WorldPos.xyz, 1.0);
          vec4 ShadowPos[NUMCASCADES];
          for (int i = 0; i < NUMCASCADES; i++)
            ShadowPos[i] = GetShadowPos(i, projWorldPos);
          diff *= GetShadow(ShadowPos, WorldPos.w);
        #endif
    
        #if defined(SPOTLIGHT)
          vec4 SpotPos = vec4(WorldPos.xyz, 1.0) * cLightMatrices[0];
          lightColor = SpotPos.w > 0.0 ? texture2DProj(sLightSpotMap, SpotPos).rgb * cLightColor.rgb : vec3(0.0, 0.0, 0.0);
        // TODO: Add spotlight support
        /*#elif defined(CUBEMASK)
            lightColor = textureCube(sLightCubeMap, vCubeMaskVec).rgb * cLightColor.rgb;*/
        #else
            lightColor = cLightColor.rgb;
        #endif
    
        #ifdef SPECULAR
            float spec = GetSpecular(normal, cCameraPosPS - WorldPos.xyz, lightDir, cMatSpecColor.a);
            finalColor = diff * lightColor * (diffColor.rgb + spec * specColor * cLightColor.a);
        #else
            finalColor = diff * lightColor * diffColor.rgb;
        #endif

        #ifdef AMBIENT
            finalColor += cAmbientColor * diffColor.rgb;
            finalColor += cMatEmissiveColor;
            gl_FragColor = vec4(GetFog(finalColor, fogFactor), diffColor.a);
        #else
            gl_FragColor = vec4(GetLitFog(finalColor, fogFactor), diffColor.a);
        #endif
    // TODO: Implement deffered shading and light prepass mode
  //  #elif defined(PREPASS)
  //      // Fill light pre-pass G-Buffer
  //      float specPower = cMatSpecColor.a / 255.0;
  //
  //      gl_FragData[0] = vec4(normal * 0.5 + 0.5, specPower);
  //      gl_FragData[1] = vec4(EncodeDepth(vWorldPos.w), 0.0);
    #elif defined(DEFERRED)
        // Fill deferred G-buffer
        float specIntensity = specColor.g;
        float specPower = cMatSpecColor.a / 255.0;
  
        vec3 finalColor = gVertexLight * diffColor.rgb;

        // TODO: add AO
        //#ifdef AO
        //    // If using AO, the vertex light ambient is black, calculate occluded ambient here
        //    finalColor += texture2D(sEmissiveMap, vTexCoord2).rgb * cAmbientColor * diffColor.rgb;
        //#endif
  
        // TODO: add EnvCubeMao
        //#ifdef ENVCUBEMAP
        //    finalColor += cMatEnvMapColor * textureCube(sEnvCubeMap, reflect(vReflectionVec, normal)).rgb;
        //#endif

        #ifdef EMISSIVEMAP
            //finalColor += cMatEmissiveColor * texture2D(sEmissiveMap, vTexCoord.xy).rgb;
        #else
            finalColor += cMatEmissiveColor;
        #endif
  
        gl_FragData[0] = vec4(GetFog(finalColor, fogFactor), 1.0);
        gl_FragData[1] = fogFactor * vec4(diffColor.rgb, specIntensity);
        gl_FragData[2] = vec4(normal * 0.5 + 0.5, specPower);
        gl_FragData[3] = vec4(EncodeDepth(WorldPos.w), 0.0);
    #else
        // Ambient & per-vertex lighting
        vec3 finalColor = gVertexLight * diffColor.rgb;

        // TODO: Add AO
        /*#ifdef AO
            // If using AO, the vertex light ambient is black, calculate occluded ambient here
            finalColor += texture2D(sEmissiveMap, vTexCoord2).rgb * cAmbientColor * diffColor.rgb;
        #endif*/
        
        // TODO: add material pass for deffered lighting
        /*#ifdef MATERIAL
            // Add light pre-pass accumulation result
            // Lights are accumulated at half intensity. Bring back to full intensity now
            vec4 lightInput = 2.0 * texture2DProj(sLightBuffer, vScreenPos);
            vec3 lightSpecColor = lightInput.a * lightInput.rgb / max(GetIntensity(lightInput.rgb), 0.001);

            finalColor += lightInput.rgb * diffColor.rgb + lightSpecColor * specColor;
        #endif*/

        // TODO: Add env cubemap
        /*#ifdef ENVCUBEMAP
            finalColor += cMatEnvMapColor * textureCube(sEnvCubeMap, reflect(vReflectionVec, normal)).rgb;
        #endif*/

        // TODO: Add emissive map
        /*#ifdef EMISSIVEMAP
            finalColor += cMatEmissiveColor * texture2D(sEmissiveMap, vTexCoord.xy).rgb;
        #else*/
            finalColor += cMatEmissiveColor;
        // #endif

        gl_FragColor = vec4(GetFog(finalColor, fogFactor), diffColor.a);
    #endif

//#endif
//
///*  // Get material diffuse albedo
//#ifdef DIFFMAP
//  vec4 diffColor = cMatDiffColor * texture2D(sDiffMap, gTexCoord);
//#ifdef ALPHAMASK
//  if (diffColor.a < 0.5)
//    discard;
//#endif
//#else
//  vec4 diffColor = cMatDiffColor;
//#endif
//
//#ifdef VERTEXCOLOR
//  diffColor *= gColor;
//#endif
//
//  // Get fog factor
//#ifdef HEIGHTFOG
//  float fogFactor = GetHeightFogFactor(gWorldPos.w, gWorldPos.y);
//#else
//  float fogFactor = GetFogFactor(gWorldPos.w);
//#endif
//
//#if defined(PREPASS)
//  // Fill light pre-pass G-Buffer
//  gl_FragData[0] = vec4(0.5, 0.5, 0.5, 1.0);
//  gl_FragData[1] = vec4(EncodeDepth(gWorldPos.w), 0.0);
//#elif defined(DEFERRED)
//  gl_FragData[0] = vec4(GetFog(diffColor.rgb, fogFactor), diffColor.a);
//  gl_FragData[1] = vec4(0.0, 0.0, 0.0, 0.0);
//  gl_FragData[2] = vec4(0.5, 0.5, 0.5, 1.0);
//  gl_FragData[3] = vec4(EncodeDepth(gWorldPos.w), 0.0);
//#else
//#ifndef MRT1
//  gl_FragColor = vec4(GetFog(diffColor.rgb, fogFactor), diffColor.a);
//#else
//  gl_FragData[0] = vec4(GetFog(diffColor.rgb, fogFactor), diffColor.a);
//  gl_FragData[1] = vec4(1.0);
//#endif
//#endif*/
}

#endif
