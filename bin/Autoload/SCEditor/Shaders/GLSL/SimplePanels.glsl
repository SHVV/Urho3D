#include "Uniforms.glsl"
#include "Transform.glsl"

#ifndef COMPILEGS
#include "Samplers.glsl"
#include "ScreenPos.glsl"
#include "Lighting.glsl"
#include "Constants.glsl"
#include "Fog.glsl"
#include "PBR.glsl"
#include "IBL.glsl"
#endif
#line 30013

#ifdef COMPILEVS
out vec3 gNormal;
#endif

void VS()
{
  mat4 modelMatrix = iModelMatrix;
  gl_Position = vec4(GetWorldPos(modelMatrix), 1.0);
  gNormal = GetWorldNormal(modelMatrix);
}

#ifdef COMPILEGS

layout(triangles) in;
in vec3 gNormal[3];

layout(triangle_strip, max_vertices = 3) out;
out vec4 vWorldPos;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBiNormal;
out vec2 vTextureCoord;
out vec3 vBaricentric;
//out vec3 vColor;
//out float vFade;

uniform mat4 cViewProj;
uniform mat4 cView;
uniform vec4 cDepthMode;

float GetDepth(vec4 clipPos)
{
  return dot(clipPos.zw, cDepthMode.zw);
}

void plate()
{
  vec3 edge1 = (gl_in[1].gl_Position - gl_in[0].gl_Position).xyz;
  vec3 edge2 = (gl_in[2].gl_Position - gl_in[0].gl_Position).xyz;
  vec3 normal = normalize(cross(edge1, edge2));
  vec3 tangent = normalize(edge2);
  vec3 binorm = normalize(cross(normal, tangent));

  const vec3 baricentric[3] = vec3[](
    vec3(1., 0., 0.),
    vec3(0., 1., 0.),
    vec3(0., 0., 1.)
  );
  const vec2 uvs[3] = vec2[](
    vec2(0, 1.0),
    vec2(0.5, 0.0),
    vec2(1.0, 1.0)
  );

  for (int i = 0; i < 3; ++i) {
    vec3 world_pos = gl_in[i].gl_Position.xyz;
    gl_Position = vec4(world_pos, 1.0) * cViewProj;
    vWorldPos = vec4(world_pos, GetDepth(gl_Position));
    //vNormal = normal;
    vNormal = gNormal[i];
    vTangent = tangent;
    //vBiNormal = binorm;
    vBiNormal = normalize(cross(normal, tangent));
    vBaricentric = baricentric[i];
    vTextureCoord = uvs[i];
    EmitVertex();
  }
  EndPrimitive();
}

void GS()
{
  plate();
}

#endif

#ifdef COMPILEPS
in vec4 vWorldPos;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBiNormal;
in vec3 vBaricentric;
in vec2 vTextureCoord;
//in vec3 vColor;
//in float vFade;
uniform float cRoughnessClose;
#endif

void PS()
{
#ifdef SHADOW_MAP
  gl_FragColor = vec4(1.0);
#else
    // Get material diffuse albedo
    #ifdef DIFFMAP
        vec4 diffInput = texture2D(sDiffMap, vTextureCoord);
        #ifdef ALPHAMASK
            if (diffInput.a < 0.5)
                discard;
        #endif
        vec4 diffColor = cMatDiffColor * diffInput;
    #else
        vec4 diffColor = cMatDiffColor;
    #endif

    vec4 norm_input = texture2D(sNormalMap, vTextureCoord);
    //#ifdef METALLIC
    //    vec4 roughMetalSrc = texture2D(sSpecMap, vTexCoord.xy);

    //    float roughness = roughMetalSrc.r + cRoughnessClose;
    //    float metalness = roughMetalSrc.g + cMetallic;
    //#else
    float roughness = cRoughness + norm_input.a;
    float metalness = cMetallic * diffInput.a;
    //#endif

    roughness *= roughness;

    roughness = clamp(roughness, ROUGHNESS_FLOOR, 1.0);
    metalness = clamp(metalness, METALNESS_FLOOR, 1.0);

    vec3 specColor = mix(0.08 * cMatSpecColor.rgb, diffColor.rgb, metalness);
    diffColor.rgb = diffColor.rgb - diffColor.rgb * metalness;

    // Get normal
    //#if defined(NORMALMAP) || defined(DIRBILLBOARD) || defined(IBL)
    //    vec3 tangent = vTangent.xyz;
    //    vec3 bitangent = vec3(vTexCoord.zw, vTangent.w);
    //    mat3 tbn = mat3(tangent, bitangent, vNormal);
    //#endif

    //#ifdef NORMALMAP
    //    vec3 nn = DecodeNormal(texture2D(sNormalMap, vTexCoord.xy));
    //    //nn.rg *= 2.0;
    //    vec3 normal = normalize(tbn * nn);
    //#else
    mat3 tbn = mat3(vTangent, vBiNormal, vNormal);
    vec3 nn = DecodeNormal(norm_input);
    vec3 normal = normalize(tbn * nn);
    //vec3 normal = normalize(vNormal);
    //#endif

    // Get fog factor
    #ifdef HEIGHTFOG
        float fogFactor = GetHeightFogFactor(vWorldPos.w, vWorldPos.y);
    #else
        float fogFactor = GetFogFactor(vWorldPos.w);
    #endif

    // Ambient & per-vertex lighting
        vec3 finalColor = vec3(0.0);//diffColor.rgb;// *vColor;

    vec3 toCamera = normalize(vWorldPos.xyz - cCameraPosPS);
    vec3 reflection = normalize(reflect(toCamera, normal));
    vec3 tangent = normalize(cross(normal, vec3(0, 1, 0)));
    vec3 bitangent = cross(tangent, normal);

    vec3 cubeColor = vec3(1.0);

    #ifdef IBL
      vec3 iblColor = ImageBasedLighting(reflection, tangent, bitangent, normal, toCamera, diffColor.rgb, specColor.rgb, roughness, cubeColor);
      float gamma = 0.0;
      finalColor.rgb += iblColor;
    #endif

    #ifdef EMISSIVEMAP
        finalColor += cMatEmissiveColor * texture2D(sEmissiveMap, vTexCoord.xy).rgb;
    #else
        finalColor += cMatEmissiveColor;
    #endif

    #ifdef DEFERRED
        // Fill deferred G-buffer
        const vec3 spareData = vec3(0, 0, 0); // Can be used to pass more data to deferred renderer
        gl_FragData[0] = vec4(specColor, spareData.r);
        gl_FragData[1] = vec4(diffColor.rgb, spareData.g);
        gl_FragData[2] = vec4(normal * roughness, spareData.b);
        gl_FragData[3] = vec4(GetFog(finalColor, fogFactor), diffColor.a);//vec4(EncodeDepth(vWorldPos.w), 0.0);
    #else
        gl_FragColor = vec4(GetFog(finalColor, fogFactor), diffColor.a);
    #endif
#endif
}
