@include "version.inc.glsl"
@include "common.inc.glsl"
@include "uniforms.inc.glsl"

in INTERFACE
{
    vec4 fragPos;
    vec2 texCoord;
    vec4 color;
    vec4 tangent;
    vec3 normal;
} In;


out vec4 fragColor;

struct input_t {
    vec3 normal;
    mat3 tbn;
};

layout(binding = IMU_DIFFUSE)   uniform sampler2D tDiffuse;
layout(binding = IMU_NORMAL)    uniform sampler2D tNormal;
layout(binding = IMU_AORM)      uniform sampler2D tAORM;
layout(binding = IMU_EMMISIVE)  uniform sampler2D tEmissive;

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

    vec4 color;
    uint debflags = floatBitsToInt(ubo.debugFlags.x);
    
    if (debflags == 0)
    {
        color = texture(tDiffuse, In.texCoord);
    }
    else if ((debflags & 1) == 1)
    {
        color = texture(tNormal, In.texCoord);
    }
    else if ((debflags & 2) == 2)
    {
        color = texture(tAORM, In.texCoord);
    }
    else if ((debflags & 4) == 4)
    {
        color = texture(tEmissive, In.texCoord);
    }

    fragColor = color * In.color * ubo.matDiffuseFactor;
}