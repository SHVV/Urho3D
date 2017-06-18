#include "Uniforms.glsl"
#include "Transform.glsl"
#include "Samplers.glsl"
#include "ScreenPos.glsl"
#include "Lighting.glsl"
#include "Constants.glsl"
#include "Fog.glsl"
#include "PBR.glsl"
#include "IBL.glsl"
#include "TriPlanarMapping.glsl"
#line 30010

varying vec3 vNormal;
varying vec4 vWorldPos;
varying vec3 vLocalNormal;
varying vec4 vLocalPos;

#ifdef COMPILEVS
attribute float iCustom2; // Scale
flat out mat3 vNormalMatrix;
flat out float vScale;
#endif

void VS()
{
  mat4 modelMatrix = iModelMatrix;
  vec3 worldPos = GetWorldPos(modelMatrix);
  gl_Position = GetClipPos(worldPos);
  vNormal = GetWorldNormal(modelMatrix);
  vWorldPos = vec4(worldPos, GetDepth(gl_Position));

  vLocalPos = iPos;
  vLocalNormal = iNormal;
  vNormalMatrix = GetNormalMatrix(modelMatrix);
  vScale = iCustom2;
}


#ifdef COMPILEPS

flat in float vScale;
flat in mat3 vNormalMatrix;

#endif

void PS()
{
#ifdef SHADOW_MAP
  gl_FragColor = vec4(1.0);
#else

  // Get material diffuse albedo
  //#ifdef DIFFMAP
  //vec4 diffInput = texture2D(sDiffMap, vTexCoord.xy);
  //    #ifdef ALPHAMASK
  //        if (diffInput.a < 0.5)
  //            discard;
  //    #endif
  //    vec4 diffColor = cMatDiffColor * diffInput;
  //#else
  //    vec4 diffColor = cMatDiffColor;
  //#endif
  //vec4 diffColor = cMatDiffColor * blended_color;

  //#ifdef METALLIC
  //  //vec4 roughMetalSrc = texture2D(sSpecMap, vTexCoord.xy);

  //  float roughness = blended_attributes.r + cRoughness;
  //  float metalness = blended_attributes.g * cMetallic;
  //#else
  //  float roughness = cRoughness;
  //  float metalness = cMetallic;
  //#endif
  //vec3 cells = fract(vBaricentric * 12 - 0.05) - 0.9;
  //cells = max(vec3(0), cells);
  //roughness *= dot(cells, cells) > 0 ? 1.2 : 1.0;
  //roughness = mix(cRoughness, roughness, vFade);

  TriPlanarResult mapped = tri_planar_map(vLocalPos.xyz / vScale, vLocalNormal);
  // TODO: factor out additional scale
  TriPlanarResult mapped2 = tri_planar_map(vLocalPos.xyz / vScale * 7.654321, vLocalNormal);

  vec4 diffColor = (mapped.diff + mapped2.diff) * cMatDiffColor * 0.5;
  float roughness = (mapped.prop.r * mapped2.prop.r) * cRoughness * 4.0;
  float metalness = (mapped.prop.g * mapped2.prop.g) * cMetallic * 4.0;
  vec3 norm = normalize(vLocalNormal + (mapped.norm + mapped2.norm) * 0.5);


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

    #ifdef NORMALMAP
      // Apply bump vector to vertex-interpolated normal vector.  
      //float3 N_for_lighting = normalize(N_orig + blended_bump);
      //vec3 normal = normalize((vLocalNormal + blended_bump_vec) * vNormalMatrix);
      //vec3 nn = DecodeNormal(texture2D(sNormalMap, vTexCoord.xy));
      //nn.rg *= 2.0;
      //vec3 normal = normalize(tbn * nn);
      vec3 normal = normalize(norm * vNormalMatrix);
    #else
        vec3 normal = normalize(vNormal);
    #endif

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
      vec3 iblColor = ImageBasedLighting(reflection, tangent, bitangent, normal, toCamera, diffColor.rgb, specColor.rgb, roughness + 0.1, cubeColor);
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
