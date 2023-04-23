@include "version.inc.glsl"

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_TexCoord;
layout(location = 2) in vec3 in_Normal;
layout(location = 3) in vec4 in_Tangent;
layout(location = 4) in vec4 in_Color;

out INTERFACE
{
    vec4 fragPos;
    vec4 color;
    vec4 normal;
} Out;

@include "uniforms.inc.glsl"

void main()
{
    gl_Position = ubo.worldToviewProjectionMatrix * ubo.localToWorldMatrix * in_Position;

    Out.fragPos = ubo.localToWorldMatrix * in_Position;
    Out.color = in_Color;
    Out.normal = vec4( in_Normal, 0.0 ) * mat4( transpose( mat3( ubo.localToWorldMatrix ) ) );
}