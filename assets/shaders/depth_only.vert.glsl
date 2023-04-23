@include "version.inc.glsl"

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_TexCoord;
layout(location = 2) in vec3 in_Normal;
layout(location = 3) in vec4 in_Tangent;
layout(location = 4) in vec4 in_Color;

@include "uniforms.inc.glsl"

void main()
{
    gl_Position = ubo.worldToviewProjectionMatrix * ubo.localToWorldMatrix * in_Position;
}