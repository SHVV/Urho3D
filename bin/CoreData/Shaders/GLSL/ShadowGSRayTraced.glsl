#include "Uniforms.glsl"
#include "Transform.glsl"

#ifndef COMPILEGS //ONLY IF NOT GS Shader

#include "Samplers.glsl"
#include "ScreenPos.glsl"
#include "Constants.glsl"

#endif //ONLY IF NOT GS Shader

uniform float cRadius;

//---------------------------------------------VS-----------------------------------------------------
#ifdef COMPILEVS

void VS()
{
  mat4 modelMatrix = iModelMatrix;
  vec4 WorldPos = iPos * modelMatrix;
  gl_Position = WorldPos * cView;
}
#endif // COMPILEVS

//---------------------------------------------GS-----------------------------------------------------
#ifdef COMPILEGS

#line 30028

uniform vec4 cDepthMode;
uniform mat4 cProj;

#ifdef BEAMS
layout(lines) in;
#define NUM_INPUT 2
#else
layout(points) in;
#define NUM_INPUT 1
#endif

#define OUTPUT_SIZE 4
layout(triangle_strip, max_vertices = 4) out;

out vec2 gUV;

void GS()
{
#ifdef BEAMS
  bool swap = gl_in[0].gl_Position.z > gl_in[1].gl_Position.z;
  vec4 iPos = gl_in[swap ? 1 : 0].gl_Position; // closer to camera
  vec4 iPos2 = gl_in[swap ? 0 : 1].gl_Position;
#else
  vec4 iPos = gl_in[0].gl_Position;
#endif

  bool ortographic = cDepthMode.x == 1.0;
  vec3 toCamera = ortographic ? vec3(0.0, 0.0, -1.0) : - normalize(iPos.xyz);
#ifdef BEAMS
  vec3 beam_dir = iPos2.xyz - iPos.xyz;
  vec3 up = normalize(cross(beam_dir, toCamera));
  // TODO: check for parallel case
  vec3 right = normalize(cross(up, beam_dir));
#else
  vec3 up = vec3(0.0, 1.0, 0.0);
  vec3 right = ortographic ? vec3(1.0, 0.0, 0.0) : normalize(cross(toCamera, up));
  if (!ortographic) {
    up = normalize(cross(right, toCamera));
  }
#endif
  float size = cRadius * 1;

  vec3 quad[OUTPUT_SIZE];
#ifdef BEAMS
  quad[0] = iPos.xyz - up * size;
  quad[1] = iPos.xyz + up * size;
  quad[2] = iPos2.xyz - up * size;
  quad[3] = iPos2.xyz + up * size;
#else
  quad[0] = iPos.xyz - (right + up) * size;
  quad[1] = iPos.xyz - (right - up) * size;
  quad[2] = iPos.xyz + (right - up) * size;
  quad[3] = iPos.xyz + (right + up) * size;
#endif
  vec2 uv[OUTPUT_SIZE];
  uv[0] = vec2(-1, -1);
  uv[1] = vec2(-1, 1);
  uv[2] = vec2(1, -1);
  uv[3] = vec2(1, 1);

  for (int i = 0; i < OUTPUT_SIZE; i++) {
    vec4 quadVertexWp = vec4(quad[i].xyz, iPos.w);
    gl_Position = quadVertexWp * cProj;
    gUV = uv[i];
    EmitVertex();
  }
  EndPrimitive();
}
#endif // COMPILEGS

//---------------------------------------------PS-----------------------------------------------------
#ifdef COMPILEPS

#line 10105

in vec2 gUV;

void PS()
{
  if (dot(gUV, gUV) > 1) {
    discard;
  }
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
}

#endif
