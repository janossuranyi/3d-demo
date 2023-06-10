@include "version.inc.glsl"
@include "defs.inc"
@include "vertex_uniforms.inc.glsl"
@include "light_uniforms.inc.glsl"

layout(location = 0) in vec4 in_Position;

void main()
{
    vec4 positionVS = gViewMatrix * gModelMatrix * in_Position;
    gl_Position =  lightData.lightProjMatrix * positionVS;
}