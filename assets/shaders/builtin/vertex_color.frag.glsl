@include "version.inc.glsl"

in INTERFACE
{
    vec4 fragPos;
    vec4 color;
    vec4 tangent;
    vec3 normal;
} In;

@include "uniforms.inc.glsl"

out vec4 fragColor;

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

    fragColor = In.color;
}