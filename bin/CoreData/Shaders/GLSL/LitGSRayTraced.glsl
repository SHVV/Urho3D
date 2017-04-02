#ifndef COMPILEGS //ONLY IF NOT GS Shader

#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "Constants.glsl"
#include "Fog.glsl"
#include "Lighting.glsl"

#ifdef PBR
#include "PBR.glsl"
#include "IBL.glsl"
#endif

#endif //ONLY IF NOT GS Shader

uniform float cRadius;
uniform float cTaper;

//---------------------------------------------VS-----------------------------------------------------
#ifdef COMPILEVS

attribute float iCustom; // radius

#ifdef VERTEXCOLOR
out vec4 vColor;
#endif
varying vec3 vVertexLight;
out float vRadius;

void VS()
{
  mat4 modelMatrix = iModelMatrix;
  vec4 WorldPos = iPos * modelMatrix;
  gl_Position = WorldPos * cView;
  vVertexLight = GetAmbient(GetZonePos(WorldPos.xyz));
  vRadius = iCustom;
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
#line 30052

uniform vec2 cGBufferInvSize;
uniform mat4 cModel;
uniform mat4 cViewProj;
uniform mat4 cViewInv;
uniform vec4 cDepthMode;
uniform vec3 cCameraPos;
uniform vec3 cCameraRot;
uniform mat4 cProj;

#ifdef BEAMS
layout(lines) in;
layout(triangle_strip, max_vertices = 6) out;
#define NUM_INPUT 2
flat out mat4 gCylinder;
//flat out mat4 gCone1;
//flat out mat4 gCone2;
#else
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;
#define NUM_INPUT 1
#endif

#ifdef VERTEXCOLOR
  in vec4 vColor[NUM_INPUT];
  out vec4 gColor;
#endif

flat out vec3 gWorldPos;
flat out vec4 gNormal;

in vec3 vVertexLight[NUM_INPUT];
out vec3 gVertexLight;

in float vRadius[NUM_INPUT];
flat out float gRadius;

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
#ifdef BEAMS
#define OUTPUT_SIZE 6
  bool swap = gl_in[0].gl_Position.z > gl_in[1].gl_Position.z;
  vec4 iPos = gl_in[swap ? 1 : 0].gl_Position; // closer to camera
  vec4 iPos2 = gl_in[swap ? 0 : 1].gl_Position;
#else
#define OUTPUT_SIZE 4
  vec4 iPos = gl_in[0].gl_Position;
#endif

  bool ortographic = cDepthMode.x == 1.0;
  vec3 toCamera = ortographic ? vec3(0.0, 0.0, -1.0) : - normalize(iPos.xyz);
#ifdef BEAMS
  float radius = min(vRadius[0], vRadius[1]) * 0.7;
  vec3 beam_dir = iPos2.xyz - iPos.xyz;

  // Limit thickness of beam
  float beam_len = length(beam_dir);
  radius = min(beam_len / 15, radius);

  vec3 up = normalize(cross(beam_dir, toCamera));
  // TODO: check for parallel case
  vec3 right = normalize(cross(up, beam_dir));
  mat4 trans = mat4(
    vec4(normalize(beam_dir), 0.0),
    vec4(up, 0.0),
    vec4(right, 0.0),
    vec4(iPos.xyz + beam_dir * 0.5, 1.0)
  );
//  mat4 trans2 = mat4(
//    vec4(normalize(beam_dir), 0.0),
//    vec4(up, 0.0),
//    vec4(right, 0.0),
//    vec4(iPos2.xyz, 1.0)
//  );
//#ifdef SHADOW_MAP
//  if (ortographic) {
//    trans[3].z = 0.0;
//    trans2[3].z = iPos2.z - iPos.z;
//  }
//#endif
  trans = inverse(trans);
  //trans2 = inverse(trans2);
  mat4 inv_trans = transpose(trans);
  //mat4 inv_trans2 = transpose(trans2);

  //mat4 cylinder = mat4(
  //  0.0, 0.0, 0.0, 0.0,
  //  0.0, 1.0, 0.0, 0.0,
  //  0.0, 0.0, 1.0, 0.0,
  //  0.0, 0.0, 0.0, -cRadius * cRadius
  //);
  float dist = distance(iPos2, iPos);
  dist = dist / 2.0;
  float cx = 1.0 / (dist * dist);
  float cyz = 1.0 / (radius * radius);
  mat4 cylinder = mat4(
    cx, 0.0, 0.0, 0.0,
    0.0, cyz, 0.0, 0.0,
    0.0, 0.0, cyz, 0.0,
    0.0, 0.0, 0.0, -1
  );

  //float taper = 0.1; // Factor out const
  //mat4 cone = mat4(
  //  -cTaper * cTaper, 0.0, 0.0, 0.0,
  //  0.0, 1.0, 0.0, 0.0,
  //  0.0, 0.0, 1.0, 0.0,
  //  0.0, 0.0, 0.0, 0.0
  //);

  mat4 trans_cylinder = inv_trans * cylinder * trans;
  //mat4 trans_cone1 = inv_trans * cone * trans;
  //mat4 trans_cone2 = inv_trans2 * cone * trans2;
#else
  float radius = vRadius[0];
  vec3 up = vec3(0.0, 1.0, 0.0);
  vec3 right = ortographic ? vec3(1.0, 0.0, 0.0) : normalize(cross(toCamera, up));
  if (!ortographic) {
    up = normalize(cross(right, toCamera));
  }
#endif

  float size = radius;

  vec4 r_calc = vec4(radius, radius, iPos.z, 1.0);
  r_calc = r_calc * cProj;
  float rx = abs(r_calc.x / (r_calc.w * cGBufferInvSize.x));
  //size = rx;
//#ifdef ALPHA
//  #ifdef BEAMS
//    if (rx > 3) return;
//  #else 
//    if (rx > 4) return;
//  #endif
//#else
//  #ifdef BEAMS
//    if (rx < 3) return;
//  #else 
//    if (rx < 4) return;
//  #endif
//#endif

#if !defined(SHADOW_MAP) && !defined(BEAMS)
  toCamera *= size;
#else
  toCamera = vec3(0);
#endif
  //size *= 5;
  vec3 quad[OUTPUT_SIZE];
  quad[0] = iPos.xyz - (right + up) * size;
  quad[1] = iPos.xyz - (right - up) * size;
  quad[2] = iPos.xyz + (right - up) * size;
  quad[3] = iPos.xyz + (right + up) * size;
#ifdef BEAMS
  quad[4] = iPos2.xyz + (right - up) * size;
  quad[5] = iPos2.xyz + (right + up) * size;
#endif

  for (int i = 0; i < OUTPUT_SIZE; i++) {
    vec4 quadVertexWp = vec4(quad[i].xyz + toCamera, iPos.w);
    gl_Position = quadVertexWp * cProj;
#ifdef SHADOW_MAP
    if (ortographic) {
      gWorldPos = vec3(iPos.xy, 0);
    } else {
      gWorldPos = vec3(iPos.xyz);
    }
#else 
    gWorldPos = vec3(iPos.xyz);
#endif

#ifdef BEAMS
    gNormal = vec4(right, iPos.z);
    gCylinder = trans_cylinder;
    //gCone1 = trans_cone1;
    //gCone2 = trans_cone2;
#else
    gNormal = vec4(toCamera, iPos.z);
#endif

#ifdef VERTEXCOLOR
    gColor = vColor[0];
#endif
    vec4 WorldPos = vec4(iPos.xyz, 1.0) * cViewInv;
    gVertexLight = vVertexLight[0];
    gRadius = radius;
    EmitVertex();
  }
  EndPrimitive();
}
#endif // COMPILEGS

//---------------------------------------------PS-----------------------------------------------------
#ifdef COMPILEPS

#line 10242

flat in vec3 gWorldPos;
flat in vec4 gNormal;
flat in float gRadius;
#ifdef BEAMS
flat in mat4 gCylinder;
//flat in mat4 gCone1;
//flat in mat4 gCone2;
#endif

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

#ifdef BEAMS
struct Ray
{
  vec3 o;		// Origin
  vec3 d;		// Direction
};

struct Hit
{
  float t;	// ray solution
  vec3 n;		// normal
  int m;		// material
};
const Hit noHit = Hit(1e10, vec3(0.), -1);

Hit intersectQuadric(mat4 q, Ray r)
{
  vec4 C = vec4(r.o, 1.);
  vec4 AC = q * C;
  vec4 D = vec4(r.d, 0.);
  vec4 AD = q * D;
  float a = dot(D, AD);
  float b = dot(C, AD) + dot(D, AC);
  float c = dot(C, AC);
  float d = b * b - 4.0 * a * c;
  if (d >= 0.0) {
    float ds = sqrt(d);
    float t0 = ((-b + ds) / (2.0*a));
    float t1 = ((-b - ds) / (2.0*a));
    float tx = max(t1, t0);
    float tn = min(t1, t0);
    /*if (tn < 0.) tn = tx;
    if (tn < 0.) {
      return noHit;
    }*/
    vec4 pos = C + tn * D;
    vec3 norm = normalize((q * pos).xyz);
    if (dot(norm, r.d) > 0) {
      pos = C + tx * D;
      norm = normalize((q * pos).xyz);
      return Hit(tx, norm, 0);
    } else {
      return Hit(tn, norm, 0);
    }
  } else {
    return noHit;
  }
}

//Hit intersect_beam(Ray r)
//{
//  Hit hit_cylinder = intersectQuadric(gCylinder, r);
//  if (hit_cylinder.m != -1) {
//    vec4 pos = vec4(r.o + hit_cylinder.t * r.d, 1.0);
//    bool in_cone1 = dot(pos, gCone1 * pos) <= 0.0;
//    bool in_cone2 = dot(pos, gCone2 * pos) <= 0.0;
//    if (in_cone1 && in_cone2) {
//      return hit_cylinder;
//    }
//  }
//
//  Hit hit1 = intersectQuadric(gCone1, r);
//  if (hit1.m != -1) {
//    vec4 pos = vec4(r.o + hit1.t * r.d, 1.0);
//    hit1.m = (dot(pos, gCylinder * pos) <= 0.0) ? 0 : -1;
//  }
//  Hit hit2 = intersectQuadric(gCone2, r);
//  if (hit2.m != -1) {
//    vec4 pos = vec4(r.o + hit2.t * r.d, 1.0);
//    hit2.m = (dot(pos, gCylinder * pos) <= 0.0) ? 0 : -1;
//  }
//  if (hit2.m != -1 && (hit2.t < hit1.t || hit1.m == -1)) {
//    return hit2;
//  }
//
//  return hit1;
//}
//
//Hit intersect_beam_old(Ray r)
//{
//  Hit hit = intersectQuadric(gCone1, r);
//  //bool in_cone1 = dot(r.o, gCone1 * r.o) < 0.0;
//  Hit hit1 = intersectQuadric(gCone2, r);
//  //bool in_cone2 = dot(r.o, gCone2 * r.o) < 0.0;
//  if (hit1.m != -1 && hit1.t > hit.t) {
//    hit = hit1;
//  } else {
//    vec4 pos = vec4(r.o + hit.t * r.d, 1.0);
//    if (dot(pos, gCone2 * pos) > 0.0) return noHit;
//  }
//
//  Hit hit2 = intersectQuadric(gCylinder, r);
//  if (hit2.m != -1 && hit2.t > hit.t) {
//    hit = hit2;
//  } else {
//    vec4 pos = vec4(r.o + hit.t * r.d, 1.0);
//    if (dot(pos, gCylinder * pos) > 0.0) return noHit;
//  }
//
//  return hit;
//}

#endif

void PS()
{
//#ifdef ALPHA
//  discard;
//#endif
  //return;
#ifdef SHADOW_MAP
  //return;
#endif
  bool ortho = cDepthReconstruct.x == 1.0;
  vec2 uv = ((gl_FragCoord.xy - cGBufferOffset) * cGBufferInvSize.xy - vec2(0.5, 0.5)) * 2 * cFrustumSize.xy;
  vec3 ray_vec = ortho ? vec3(0, 0, 1.0) : normalize(vec3(uv, cFrustumSize.z));
  ray_vec.y = -ray_vec.y;
  vec3 origin = ortho ? vec3(uv, 0) : vec3(0, 0, 0);
  origin.y = -origin.y;
  //vec3 ray_vec = vec3((gl_FragCoord.xy - cGBufferOffset) * cGBufferInvSize.xy, 0);
  //vec3 norm_pos = gWorldPos.xyz - origin;
  //gl_FragColor = vec4(ray_vec.xyz, 1.0);
  //return;
  //gWorldPos.z *= 1.5;

#ifdef BEAMS
  Ray ray = Ray(origin, ray_vec);
  //Hit hit = intersect_beam(ray);
  Hit hit = intersectQuadric(gCylinder, ray);
  if (hit.m == -1) {
    discard;
  }
  vec3 pos = ray_vec * hit.t + origin;
  vec3 normal = hit.n;
#else
  vec3 vew_pos = gWorldPos - origin;
  // TODO: move code into separate ray tracing function
  float a = 1;
  float b = -2 * dot(ray_vec, vew_pos);
  float c = dot(vew_pos, vew_pos) - gRadius * gRadius;// cRadius * cRadius;
  float D = b * b - 4 * a * c;
  if (D < 0) {
    discard;
  }
  float t = (-b - sqrt(D)) / (2 * a);
  vec3 pos = ray_vec * t + origin;
  vec3 normal = normalize(pos - gWorldPos);
  // End of sphere trace
#endif
  //float dist = (gNormal.w - 30) / 15;
  //dist = min(dist, 1.0);
  //dist = max(dist, 0.0);
  //normal = mix(normal, gNormal.xyz, dist);

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

    #ifdef PBR
      float roughness = cRoughness; //1.0 - cMatSpecColor.a / 255.0; //roughMetalSrc.r + cRoughness;
      float metalness = cMetallic; //roughMetalSrc.g + cMetallic;

      roughness *= roughness;

      roughness = clamp(roughness, ROUGHNESS_FLOOR, 1.0);
      metalness = clamp(metalness, METALNESS_FLOOR, 1.0);

      specColor = mix(0.08 * specColor.rgb, diffColor.rgb, metalness);
      diffColor.rgb = diffColor.rgb - diffColor.rgb * metalness;
    #endif

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

        float atten = 1;

#if defined(DIRLIGHT)
        atten = GetAtten(normal, WorldPos.xyz, lightDir);
#elif defined(SPOTLIGHT)
        atten = GetAttenSpot(normal, WorldPos.xyz, lightDir);
#else
        atten = GetAttenPoint(normal, WorldPos.xyz, lightDir);
#endif

        float shadow = 1.0;
#ifdef SHADOW
        shadow = GetShadow(vShadowPos, WorldPos.w);
#endif

#if defined(SPOTLIGHT)
        lightColor = vSpotPos.w > 0.0 ? texture2DProj(sLightSpotMap, vSpotPos).rgb * cLightColor.rgb : vec3(0.0, 0.0, 0.0);
#elif defined(CUBEMASK)
        lightColor = textureCube(sLightCubeMap, vCubeMaskVec).rgb * cLightColor.rgb;
#else
        lightColor = cLightColor.rgb;
#endif
        vec3 toCamera = normalize(cCameraPosPS - WorldPos.xyz);
        vec3 lightVec = normalize(lightDir);
        float ndl = clamp((dot(normal, lightVec)), M_EPSILON, 1.0);

        vec3 BRDF = GetBRDF(WorldPos.xyz, lightDir, lightVec, toCamera, normal, roughness, diffColor.rgb, specColor);

        finalColor.rgb = BRDF * lightColor * (atten * shadow) / M_PI;

#ifdef AMBIENT
        finalColor += cAmbientColor.rgb * diffColor.rgb;
        finalColor += cMatEmissiveColor;
        gl_FragColor = vec4(GetFog(finalColor, fogFactor), diffColor.a);
#else
        gl_FragColor = vec4(GetLitFog(finalColor, fogFactor), diffColor.a);
#endif
        //vec3 lightColor;
        //vec3 lightDir;
        //vec3 finalColor;

        //float diff = GetDiffuse(normal, WorldPos.xyz, lightDir);

        //#ifdef SHADOW
        //  vec4 projWorldPos = vec4(WorldPos.xyz, 1.0);
        //  vec4 ShadowPos[NUMCASCADES];
        //  for (int i = 0; i < NUMCASCADES; i++)
        //    ShadowPos[i] = GetShadowPos(i, projWorldPos);
        //  diff *= GetShadow(ShadowPos, WorldPos.w);
        //#endif
    
        //#if defined(SPOTLIGHT)
        //  vec4 SpotPos = vec4(WorldPos.xyz, 1.0) * cLightMatrices[0];
        //  lightColor = SpotPos.w > 0.0 ? texture2DProj(sLightSpotMap, SpotPos).rgb * cLightColor.rgb : vec3(0.0, 0.0, 0.0);
        //// TODO: Add spotlight support
        ///*#elif defined(CUBEMASK)
        //    lightColor = textureCube(sLightCubeMap, vCubeMaskVec).rgb * cLightColor.rgb;*/
        //#else
        //    lightColor = cLightColor.rgb;
        //#endif
    
        //#ifdef SPECULAR
        //    float spec = GetSpecular(normal, cCameraPosPS - WorldPos.xyz, lightDir, cMatSpecColor.a);
        //    finalColor = diff * lightColor * (diffColor.rgb + spec * specColor * cLightColor.a);
        //#else
        //    finalColor = diff * lightColor * diffColor.rgb;
        //#endif

        //#ifdef AMBIENT
        //    finalColor += cAmbientColor.rgb * diffColor.rgb;
        //    finalColor += cMatEmissiveColor;
        //    gl_FragColor = vec4(GetFog(finalColor, fogFactor), diffColor.a);
        //#else
        //    gl_FragColor = vec4(GetLitFog(finalColor, fogFactor), diffColor.a);
        //#endif
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
        //    finalColor += texture2D(sEmissiveMap, vTexCoord2).rgb * cAmbientColor.rgb * diffColor.rgb;
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
  
        #ifdef PBR
            vec3 toCamera = normalize(WorldPos.xyz - cCameraPosPS);
            vec3 reflection = normalize(reflect(toCamera, normal));
            vec3 tangent = normalize(cross(normal, vec3(0, 1, 0)));
            vec3 bitangent = cross(tangent, normal);
            vec3 cubeColor = gVertexLight.rgb;
            vec3 iblColor = ImageBasedLighting(
              reflection,
              tangent,
              bitangent,
              normal,
              toCamera,
              diffColor.rgb,
              specColor.rgb,
              roughness,
              cubeColor
            );
            float gamma = 0.0;
            finalColor.rgb += iblColor;

            const vec3 spareData = vec3(0, 0, 0); // Can be used to pass more data to deferred renderer
            gl_FragData[0] = vec4(specColor, spareData.r);
            gl_FragData[1] = vec4(diffColor.rgb, spareData.g);
            gl_FragData[2] = vec4(normal * roughness, spareData.b);
            //gl_FragData[3] = vec4(EncodeDepth(WorldPos.w), 0.0);
            gl_FragData[3] = vec4(GetFog(finalColor, fogFactor), diffColor.a);
        #else
            gl_FragData[0] = vec4(GetFog(finalColor, fogFactor), 1.0);
            gl_FragData[1] = fogFactor * vec4(diffColor.rgb, specIntensity);
            gl_FragData[2] = vec4(normal * 0.5 + 0.5, specPower);
            gl_FragData[3] = vec4(EncodeDepth(WorldPos.w), 0.0);
        #endif
    #else
        // Ambient & per-vertex lighting
        vec3 finalColor = gVertexLight * diffColor.rgb;

        // TODO: Add AO
        /*#ifdef AO
            // If using AO, the vertex light ambient is black, calculate occluded ambient here
            finalColor += texture2D(sEmissiveMap, vTexCoord2).rgb * cAmbientColor.rgb * diffColor.rgb;
        #endif*/
        
        // TODO: add material pass for deffered lighting
        /*#ifdef MATERIAL
            // Add light pre-pass accumulation result
            // Lights are accumulated at half intensity. Bring back to full intensity now
            vec4 lightInput = 2.0 * texture2DProj(sLightBuffer, vScreenPos);
            vec3 lightSpecColor = lightInput.a * lightInput.rgb / max(GetIntensity(lightInput.rgb), 0.001);

            finalColor += lightInput.rgb * diffColor.rgb + lightSpecColor * specColor;
        #endif*/

        #ifdef PBR
          vec3 toCamera = normalize(WorldPos.xyz - cCameraPosPS);
          vec3 reflection = normalize(reflect(toCamera, normal));
          vec3 tangent = normalize(cross(normal, vec3(0, 1, 0)));
          vec3 bitangent = cross(tangent, normal);
          vec3 cubeColor = gVertexLight.rgb;
          vec3 iblColor = ImageBasedLighting(
            reflection, 
            tangent, 
            bitangent, 
            normal, 
            toCamera, 
            diffColor.rgb, 
            specColor.rgb, 
            roughness, 
            cubeColor
          );
          float gamma = 0.0;
          finalColor.rgb += iblColor;
        #endif

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
