#include "version.inc.glsl"

const vec2 kPosition[] = vec2[]
(
  vec2(-1.0f, 1.0),
  vec2(-1.0f,-1.0f),
  vec2( 1.0f,-1.0f),
  vec2( 1.0f, 1.0f),
  vec2(-1.0f, 1.0f),
  vec2( 1.0f,-1.0f)
);

const vec2 kTexCoord[] = vec2[]
(
  vec2(0.0f, 1.0f),
  vec2(0.0f, 0.0f),
  vec2(1.0f, 0.0f),
  vec2(1.0f, 1.0f),
  vec2(0.0f, 1.0f),
  vec2(1.0f, 0.0f)
);

out INTERFACE {
  vec2 uv;
} Out;

uniform mat4 g_mWorldTransform = mat4(1.0);

void main()
{
  gl_Position = g_mWorldTransform * vec4(kPosition[gl_VertexID], 0.0, 1.0);
  Out.uv = kTexCoord[gl_VertexID];
}