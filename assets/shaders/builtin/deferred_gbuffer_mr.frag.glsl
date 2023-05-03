@include "version.inc.glsl"

in INTERFACE
{
    vec4 fragPos;
    vec4 color;
    vec4 tangent;
    vec2 texCoord;
    vec3 normal;
} In;

layout(location = 0) out vec3 outAlbedo;
layout(location = 1) out vec2 outNormal;
layout(location = 2) out vec3 outSpec;
layout(location = 3) out vec3 outFragPos;

layout(binding = 0) uniform sampler2D texAlbedo;
layout(binding = 1) uniform sampler2D texNormal;
layout(binding = 2) uniform sampler2D texSpecular;

// @include "uniforms.inc.glsl"

void main()
{
    outFragPos = In.fragPos.xyz;
    outAlbedo = texture(texAlbedo, In.texCoord).xyz;
    outNormal = In.normal.xy;
    outSpec = texture(texSpecular, In.texCoord).xyz;    
}