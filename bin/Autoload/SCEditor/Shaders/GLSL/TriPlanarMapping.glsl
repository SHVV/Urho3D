#ifdef COMPILEPS

struct TriPlanarResult {
  vec4 diff;
  vec3 norm;
  vec2 prop;
};

TriPlanarResult tri_planar_map(vec3 pos, vec3 norm) {
  // Determine the blend weights for the 3 planar projections.  
  // N_orig is the vertex-interpolated normal vector.  
  vec3 blend_weights = abs(norm);   // Tighten up the blending zone:  
  blend_weights = (blend_weights - 0.4) * 7;
  blend_weights = max(blend_weights, 0);      // Force weights to sum to 1.0 (very important!)  
  blend_weights /= (blend_weights.x + blend_weights.y + blend_weights.z);
  // Now determine a color value and bump vector for each of the 3  
  // projections, blend them, and store blended results in these two  
  // vectors:  
  TriPlanarResult result;
  vec4 blended_color;
  vec3 blended_bump_vec;
  vec2 blended_attributes;

  // Compute the UV coords for each of the 3 planar projections.  
  // TODO: factor out 
  // tex_scale (default ~ 1.0) determines how big the textures appear.  
  float tex_scale = 1;
  vec2 coord1 = pos.yz * tex_scale;
  vec2 coord2 = pos.zx * tex_scale;
  vec2 coord3 = pos.xy * tex_scale;

  // This is where you would apply conditional displacement mapping.  
  //if (blend_weights.x > 0) coord1 = . . .  
  //if (blend_weights.y > 0) coord2 = . . .  
  //if (blend_weights.z > 0) coord3 = . . .  

  // Sample color maps for each projection, at those UV coords.
  vec4 col1 = texture2D(sDiffMap, coord1);
  vec4 col2 = texture2D(sDiffMap, coord2);
  vec4 col3 = texture2D(sDiffMap, coord3);

  // Finally, blend the results of the 3 planar projections.  
  result.diff = col1.xyzw * blend_weights.xxxx +
                col2.xyzw * blend_weights.yyyy +
                col3.xyzw * blend_weights.zzzz;

  // Sample bump maps too, and generate bump vectors.  
  // (Note: this uses an oversimplified tangent basis.)  
  vec4 bumpFetch1 = texture2D(sNormalMap, coord1);
  vec4 bumpFetch2 = texture2D(sNormalMap, coord2);
  vec4 bumpFetch3 = texture2D(sNormalMap, coord3);

  vec3 bump1 = vec3(0.5, bumpFetch1.x, bumpFetch1.y) * 2.0 - 1.0;
  vec3 bump2 = vec3(bumpFetch2.y, 0.5, bumpFetch2.x) * 2.0 - 1.0;
  vec3 bump3 = vec3(bumpFetch3.x, bumpFetch3.y, 0.5) * 2.0 - 1.0;

  result.norm = bump1.xyz * blend_weights.xxx +
                bump2.xyz * blend_weights.yyy +
                bump3.xyz * blend_weights.zzz;

  vec4 prop1 = texture2D(sSpecMap, coord1);
  vec4 prop2 = texture2D(sSpecMap, coord2);
  vec4 prop3 = texture2D(sSpecMap, coord3);

  result.prop = prop1.xy * blend_weights.xx +
                prop2.xy * blend_weights.yy +
                prop3.xy * blend_weights.zz;

  //result.diff *= cMatDiffColor;

  //result.prop.r *= cRoughness * 2;
  //result.prop.g *= cMetallic * 2;

  //result.norm = normalize(norm + result.norm);

  return result;
}

#endif