@include "version.inc.glsl"

out vec2 texCoord;

layout(binding = 0) uniform sampler2D samp_tex0;

float GetLuma ( vec3 c ) {
	return dot( c, vec3( 0.2126, 0.7152, 0.0722 ) );
}

vec4 tex2Dlod( sampler2D image, vec4 texcoord ) { return textureLod( image, texcoord.xy, texcoord.w ); }
float saturate(float x) { return clamp(x, 0.0, 1.0); }
vec2 screenPosToTexcoord ( vec2 pos, vec4 bias_scale ) { return ( pos * bias_scale.zw + bias_scale.xy ); }

out vec4 out_FragColor0;

// 0: target offset,1/size
// 1: source w,h,1/w,1/h
uniform vec4 _fa_freqHigh[2];

void main() 
{
    vec2 texcoord = screenPosToTexcoord( gl_FragCoord.xy, _fa_freqHigh[0 ] );
    {
        vec2 sampleScale = vec2( 1.0 );
        vec2 sampleTexcoord = texcoord * sampleScale;
        vec3 c = tex2Dlod( samp_tex0, vec4( sampleTexcoord.xy, 0, 0 ) ).xyz;
        vec3 c0 = tex2Dlod( samp_tex0, vec4( sampleTexcoord.xy + sampleScale * _fa_freqHigh[1 ].zw * vec2( 0.86, 0.50 ), 0, 0 ) ).xyz;
        vec3 c1 = tex2Dlod( samp_tex0, vec4( sampleTexcoord.xy + sampleScale * _fa_freqHigh[1 ].zw * vec2( -0.50, 0.86 ), 0, 0 ) ).xyz;
        vec3 c2 = tex2Dlod( samp_tex0, vec4( sampleTexcoord.xy + sampleScale * _fa_freqHigh[1 ].zw * vec2( -0.86, -0.5 ), 0, 0 ) ).xyz;
        vec3 c3 = tex2Dlod( samp_tex0, vec4( sampleTexcoord.xy + sampleScale * _fa_freqHigh[1 ].zw * vec2( 0.50, -0.86 ), 0, 0 ) ).xyz;
        out_FragColor0.xyz = max( ( c + c0 + c1 + c2 + c3 ) * 0.2, vec3( 0.0 ) );
        //if (_fa_freqHigh[0].x != 0.0)
        {
            float l = GetLuma( out_FragColor0.xyz );
            out_FragColor0.xyz *= smoothstep( 0.0, 1.0, saturate( l ) );
        }
	}
}