@include "version.inc.glsl"
@include "defs.inc"
@include "fragment_uniforms.inc.glsl"

in vec2 texCoord;

layout(binding = 0) uniform sampler2D samp_tex0;

float GetLuma ( vec3 c ) {
	return dot( c, vec3( 0.2126, 0.7152, 0.0722 ) );
}

vec4 tex2Dlod( sampler2D image, vec4 texcoord ) { return textureLod( image, texcoord.xy, texcoord.w ); }
float saturate(float x) { return clamp(x, 0.0, 1.0); }
vec2 screenPosToTexcoord ( vec2 pos, vec4 bias_scale ) { return ( pos * bias_scale.zw + bias_scale.xy ); }

out vec4 out_FragColor0;

void main() 
{    
    vec3 c = tex2Dlod( samp_tex0, vec4( texCoord.xy, 0, 0 ) ).xyz;
    vec3 c0 = tex2Dlod( samp_tex0, vec4( texCoord.xy + gBloomParams.zw * vec2( 0.86, 0.50 ), 0, 0 ) ).xyz;
    vec3 c1 = tex2Dlod( samp_tex0, vec4( texCoord.xy + gBloomParams.zw * vec2( -0.50, 0.86 ), 0, 0 ) ).xyz;
    vec3 c2 = tex2Dlod( samp_tex0, vec4( texCoord.xy + gBloomParams.zw * vec2( -0.86, -0.5 ), 0, 0 ) ).xyz;
    vec3 c3 = tex2Dlod( samp_tex0, vec4( texCoord.xy + gBloomParams.zw * vec2( 0.50, -0.86 ), 0, 0 ) ).xyz;
    out_FragColor0.xyz = max( ( c + c0 + c1 + c2 + c3 ) * 0.2, vec3( 0.0 ) );

    if ( gBloomParams.x == 1.0 ) {
        float luma = ( GetLuma( c.xyz ) + 1e-6 );
        float luma0 = ( GetLuma( c0.xyz ) + 1e-6 );
        float luma1 = ( GetLuma( c1.xyz ) + 1e-6 );
        float luma2 = ( GetLuma( c2.xyz ) + 1e-6 );
        float luma3 = ( GetLuma( c3.xyz ) + 1e-6 );
        luma = min( gBloomParams2.x, max( ( luma + luma0 + luma1 + luma2 + luma3 ) * 0.2, 0.0 ) );
        out_FragColor0.xyz = vec3( ( luma * gBloomParams2.y != 0.0 ) ? 1.0 : luma );
    } 

    if (gBloomParams.x == 2.0)
    {
        float l = GetLuma( out_FragColor0.xyz );
        out_FragColor0.xyz *= smoothstep( 0.0, 1.0, saturate( l - gBloomParams2.y ) );
    }	
}