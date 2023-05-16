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
    vec3 tangent;
    mat3 tbn;
};

layout(binding = IMU_DIFFUSE)   uniform sampler2D tDiffuse;
layout(binding = IMU_NORMAL)    uniform sampler2D tNormal;
layout(binding = IMU_AORM)      uniform sampler2D tAORM;
layout(binding = IMU_EMMISIVE)  uniform sampler2D tEmissive;

vec3 ReconstructNormal(vec3 normalTS) { return (normalTS * 2.0 - 1.0); }
vec3 EncodeNormal(vec3 N) { return saturate( (1.0 + N) * 0.5 ); }

void main()
{
    
    input_t inputs = input_t(vec3(0.0),vec3(0.0),mat3(0.0));

    {
        vec3 localTangent       = normalize( In.tangent.xyz );
        vec3 localNormal        = normalize( In.normal );
        vec3 derivedBitangent   = normalize( cross( localNormal, localTangent ) * In.tangent.w );

        inputs.tbn      = transpose( mat3( localTangent, derivedBitangent, localNormal ) );
        inputs.normal   = localNormal;
        inputs.tangent  = localTangent;
    }

    vec4 color = vec4( 1.0 );
    int debflags = int( ubo.debugFlags.x );
    
    if ( debflags == 0 )
    {
        color = texture( tDiffuse, In.texCoord );
    }
    else if ( ( debflags & 1 ) == 1 )
    {
        vec3 normalTS = texture(tNormal, In.texCoord).xyz * 2.0 - 1.0;
        vec3 worldNormal = normalize( inputs.tbn * normalTS );
        color.xyz = EncodeNormal( worldNormal );
    }
    else if ( ( debflags & 2 ) == 2 )
    {
        color.xyz = texture(tAORM, In.texCoord).xyz;
    }
    else if ( ( debflags & 4 ) == 4 )
    {
        color.xyz = EncodeNormal( inputs.tangent  ); // texture(tEmissive, In.texCoord);
    }
    else if ( ( debflags & 8 ) == 8 )
    {
        color.xyz = EncodeNormal( In.fragPos.xyz );
    }
    else if ( ( debflags & 16 ) == 16 )
    {
        color.xyz = EncodeNormal( In.normal );
    }

    fragColor = color * In.color * ubo.matDiffuseFactor;
}