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
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outSpec;
layout(location = 3) out vec3 outFragPos;

layout(binding = 0) uniform sampler2D texAlbedo;
layout(binding = 1) uniform sampler2D texNormal;
layout(binding = 2) uniform sampler2D texSpecular;

//@include "uniforms.inc.glsl"

struct input_t {
    vec3 normal;
    mat3 tbn;
};

void main()
{
    input_t inputs = input_t(vec3(0.0),mat3(1.0));
    {
        vec3 localNormal = normalize( In.normal );
        vec3 localTangent = normalize( In.tangent.xyz );
        vec3 derivedBitangent = normalize( cross( localNormal, localTangent ) * In.tangent.w );

        inputs.tbn = mat3( localTangent, derivedBitangent, localNormal );
        inputs.normal = localNormal;
    }

    outFragPos = In.fragPos.xyz;
    outAlbedo = texture(texAlbedo, In.texCoord).xyz;
    outSpec = texture(texSpecular, In.texCoord).xyz;    
    outNormal = inputs.normal;
}