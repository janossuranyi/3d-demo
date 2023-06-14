#version 430 core
#extension GL_ARB_shader_clock : enable
void clip( float v ) { if ( v < 0.0 ) { discard; } }
void clip( vec2 v ) { if ( any( lessThan( v, vec2( 0.0 ) ) ) ) { discard; } }
void clip( vec3 v ) { if ( any( lessThan( v, vec3( 0.0 ) ) ) ) { discard; } }
void clip( vec4 v ) { if ( any( lessThan( v, vec4( 0.0 ) ) ) ) { discard; } }

float saturate( float v ) { return clamp( v, 0.0, 1.0 ); }
vec2 saturate( vec2 v ) { return clamp( v, 0.0, 1.0 ); }
vec3 saturate( vec3 v ) { return clamp( v, 0.0, 1.0 ); }
vec4 saturate( vec4 v ) { return clamp( v, 0.0, 1.0 ); }

vec4 tex2D( sampler2D image, vec2 texcoord ) { return texture( image, texcoord.xy ); }
vec4 tex2D( sampler2DShadow image, vec3 texcoord ) { return vec4( texture( image, texcoord.xyz ) ); }
vec4 tex2DARRAY( sampler2DArray image, vec3 texcoord ) { return texture( image, texcoord.xyz ); }

vec4 tex2D( sampler2D image, vec2 texcoord, vec2 dx, vec2 dy ) { return textureGrad( image, texcoord.xy, dx, dy ); }
vec4 tex2D( sampler2DShadow image, vec3 texcoord, vec2 dx, vec2 dy ) { return vec4( textureGrad( image, texcoord.xyz, dx, dy ) ); }
vec4 tex2DARRAY( sampler2DArray image, vec3 texcoord, vec2 dx, vec2 dy ) { return textureGrad( image, texcoord.xyz, dx, dy ); }

vec4 texCUBE( samplerCube image, vec3 texcoord ) { return texture( image, texcoord.xyz ); }
vec4 texCUBE( samplerCubeShadow image, vec4 texcoord ) { return vec4( texture( image, texcoord.xyzw ) ); }
vec4 texCUBEARRAY( samplerCubeArray image, vec4 texcoord ) { return texture( image, texcoord.xyzw ); }

vec4 tex1Dproj( sampler1D image, vec2 texcoord ) { return textureProj( image, texcoord ); }
vec4 tex2Dproj( sampler2D image, vec3 texcoord ) { return textureProj( image, texcoord ); }
vec4 tex3Dproj( sampler3D image, vec4 texcoord ) { return textureProj( image, texcoord ); }

vec4 tex1Dbias( sampler1D image, vec4 texcoord ) { return texture( image, texcoord.x, texcoord.w ); }
vec4 tex2Dbias( sampler2D image, vec4 texcoord ) { return texture( image, texcoord.xy, texcoord.w ); }
vec4 tex2DARRAYbias( sampler2DArray image, vec4 texcoord ) { return texture( image, texcoord.xyz, texcoord.w ); }
vec4 tex3Dbias( sampler3D image, vec4 texcoord ) { return texture( image, texcoord.xyz, texcoord.w ); }
vec4 texCUBEbias( samplerCube image, vec4 texcoord ) { return texture( image, texcoord.xyz, texcoord.w ); }
vec4 texCUBEARRAYbias( samplerCubeArray image, vec4 texcoord, float bias ) { return texture( image, texcoord.xyzw, bias); }

vec4 tex1Dlod( sampler1D image, vec4 texcoord ) { return textureLod( image, texcoord.x, texcoord.w ); }
vec4 tex2Dlod( sampler2D image, vec4 texcoord ) { return textureLod( image, texcoord.xy, texcoord.w ); }
vec4 tex2DARRAYlod( sampler2DArray image, vec4 texcoord ) { return textureLod( image, texcoord.xyz, texcoord.w ); }
vec4 tex3Dlod( sampler3D image, vec4 texcoord ) { return textureLod( image, texcoord.xyz, texcoord.w ); }
vec4 texCUBElod( samplerCube image, vec4 texcoord ) { return textureLod( image, texcoord.xyz, texcoord.w ); }
vec4 texCUBEARRAYlod( samplerCubeArray image, vec4 texcoord, float lod ) { return textureLod( image, texcoord.xyzw, lod ); }

vec4 tex2DGatherRed( sampler2D image, vec2 texcoord ) { return textureGather( image, texcoord, 0 ); }
vec4 tex2DGatherGreen( sampler2D image, vec2 texcoord ) { return textureGather( image, texcoord, 1 ); }
vec4 tex2DGatherBlue( sampler2D image, vec2 texcoord ) { return textureGather( image, texcoord, 2 ); }
vec4 tex2DGatherAlpha( sampler2D image, vec2 texcoord ) { return textureGather( image, texcoord, 3 ); }

vec4 tex2DGatherOffsetRed( sampler2D image, vec2 texcoord, const ivec2 v0 ) { return textureGatherOffset( image, texcoord, v0, 0 ); }
vec4 tex2DGatherOffsetGreen( sampler2D image, vec2 texcoord, const ivec2 v0 ) { return textureGatherOffset( image, texcoord, v0, 1 ); }
vec4 tex2DGatherOffsetBlue( sampler2D image, vec2 texcoord, const ivec2 v0 ) { return textureGatherOffset( image, texcoord, v0, 2 ); }
vec4 tex2DGatherOffsetAlpha( sampler2D image, vec2 texcoord, const ivec2 v0 ) { return textureGatherOffset( image, texcoord, v0, 3 ); }

#define tex2DGatherOffsetsRed( image, texcoord, v0, v1, v2, v3 ) textureGatherOffsets( image, texcoord, ivec2[]( v0, v1, v2, v3 ), 0 )
#define tex2DGatherOffsetsGreen( image, texcoord, v0, v1, v2, v3 ) textureGatherOffsets( image, texcoord, ivec2[]( v0, v1, v2, v3 ), 1 )
#define tex2DGatherOffsetsBlue( image, texcoord, v0, v1, v2, v3 ) textureGatherOffsets( image, texcoord, ivec2[]( v0, v1, v2, v3 ), 2 )
#define tex2DGatherOffsetsAlpha( image, texcoord, v0, v1, v2, v3 ) textureGatherOffsets( image, texcoord, ivec2[]( v0, v1, v2, v3 ), 3 )

uniform vec4 _fa_freqHigh [3];
uniform vec4 _fa_freqLow [1];
uniform sampler2D samp_tex0;

in vec4 gl_FragCoord;

out vec4 out_FragColor0;

vec2 screenPosToTexcoord ( vec2 pos, vec4 bias_scale ) { return ( pos * bias_scale.zw + bias_scale.xy ); }
vec2 screenPosToTexcoord ( vec2 pos, vec4 bias_scale, vec4 resolution_scale ) { return ( ( pos * bias_scale.zw + bias_scale.xy ) * resolution_scale.xy ); }
float GetLuma ( vec3 c ) {
	return dot( c, vec3( 0.2126, 0.7152, 0.0722 ) );
}
void main() {
	{
		vec2 texcoord = screenPosToTexcoord( gl_FragCoord.xy, _fa_freqHigh[0 ] );
		{
			vec2 sampleScale = _fa_freqHigh[1 ].y != 0.0 ? _fa_freqLow[0 ].xy : vec2( 1.0 );
			vec2 sampleTexcoord = texcoord * sampleScale;
			// _fa_freqHigh[2] = {238,124,0.0042,0.0081}
			vec3 c = tex2Dlod( samp_tex0, vec4( sampleTexcoord.xy, 0, 0 ) ).xyz;
			vec3 c0 = tex2Dlod( samp_tex0, vec4( sampleTexcoord.xy + sampleScale * _fa_freqHigh[2 ].zw * vec2( 0.86, 0.50 ), 0, 0 ) ).xyz;
			vec3 c1 = tex2Dlod( samp_tex0, vec4( sampleTexcoord.xy + sampleScale * _fa_freqHigh[2 ].zw * vec2( -0.50, 0.86 ), 0, 0 ) ).xyz;
			vec3 c2 = tex2Dlod( samp_tex0, vec4( sampleTexcoord.xy + sampleScale * _fa_freqHigh[2 ].zw * vec2( -0.86, -0.5 ), 0, 0 ) ).xyz;
			vec3 c3 = tex2Dlod( samp_tex0, vec4( sampleTexcoord.xy + sampleScale * _fa_freqHigh[2 ].zw * vec2( 0.50, -0.86 ), 0, 0 ) ).xyz;
			out_FragColor0.xyz = max( ( c + c0 + c1 + c2 + c3 ) * 0.2, vec3( 0.0 ) );
			if ( _fa_freqHigh[1 ].x == 1.0 ) {
				float luma = ( GetLuma( c.xyz ) + 1e-6 );
				float luma0 = ( GetLuma( c0.xyz ) + 1e-6 );
				float luma1 = ( GetLuma( c1.xyz ) + 1e-6 );
				float luma2 = ( GetLuma( c2.xyz ) + 1e-6 );
				float luma3 = ( GetLuma( c3.xyz ) + 1e-6 );
				luma = min( _fa_freqHigh[1 ].z, max( ( luma + luma0 + luma1 + luma2 + luma3 ) * 0.2, 0.0 ) );
				out_FragColor0.xyz = vec3( ( luma * _fa_freqHigh[1 ].w != 0.0 ) ? 1.0 : luma );
			} 
			if ( _fa_freqHigh[1 ].x == 2.0 ) {
				float l = GetLuma( out_FragColor0.xyz );
				out_FragColor0.xyz *= smoothstep( 0.0, 1.0, saturate( l - _fa_freqHigh[1 ].w ) );
			}
		};
	}
}