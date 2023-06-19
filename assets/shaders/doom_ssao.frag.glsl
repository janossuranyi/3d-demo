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

uniform vec4 _fa_freqHigh [5];
uniform vec4 _fa_freqLow [9];
uniform sharedparmsuniformbuffer_ubo { vec4 sharedparmsuniformbuffer[64]; };
uniform sampler2D samp_viewdepthmap;
uniform sampler2D samp_tex0;
uniform sampler2D samp_tex3;
uniform sampler2D samp_tex2;
uniform sampler2D samp_tex1;

in vec4 gl_FragCoord;

out vec4 out_FragColor0;

vec4 sqr ( vec4 x ) { return ( x * x ); }
vec3 sqr ( vec3 x ) { return ( x * x ); }
vec2 sqr ( vec2 x ) { return ( x * x ); }
float sqr ( float x ) { return ( x * x ); }
vec4 MatrixMul ( vec3 pos, mat4x4 mat ) {
	return vec4( ( pos.x * mat[0].x + ( pos.y * mat[0].y + ( pos.z * mat[0].z + mat[0].w ) ) ),
	( pos.x * mat[1].x + ( pos.y * mat[1].y + ( pos.z * mat[1].z + mat[1].w ) ) ),
	( pos.x * mat[2].x + ( pos.y * mat[2].y + ( pos.z * mat[2].z + mat[2].w ) ) ),
	( pos.x * mat[3].x + ( pos.y * mat[3].y + ( pos.z * mat[3].z + mat[3].w ) ) ) );
}
vec3 MatrixMul ( vec3 pos, mat3x4 mat ) {
	return vec3( ( pos.x * mat[0].x + ( pos.y * mat[0].y + ( pos.z * mat[0].z + mat[0].w ) ) ),
	( pos.x * mat[1].x + ( pos.y * mat[1].y + ( pos.z * mat[1].z + mat[1].w ) ) ),
	( pos.x * mat[2].x + ( pos.y * mat[2].y + ( pos.z * mat[2].z + mat[2].w ) ) ) );
}
vec4 MatrixMul ( vec3 pos, vec4 matX, vec4 matY, vec4 matZ, vec4 matW ) {
	return vec4( ( pos.x * matX.x + ( pos.y * matX.y + ( pos.z * matX.z + matX.w ) ) ),
	( pos.x * matY.x + ( pos.y * matY.y + ( pos.z * matY.z + matY.w ) ) ),
	( pos.x * matZ.x + ( pos.y * matZ.y + ( pos.z * matZ.z + matZ.w ) ) ),
	( pos.x * matW.x + ( pos.y * matW.y + ( pos.z * matW.z + matW.w ) ) ) );
}
vec3 MatrixMul ( vec3 pos, vec4 matX, vec4 matY, vec4 matZ ) {
	return vec3( ( pos.x * matX.x + ( pos.y * matX.y + ( pos.z * matX.z + matX.w ) ) ),
	( pos.x * matY.x + ( pos.y * matY.y + ( pos.z * matY.z + matY.w ) ) ),
	( pos.x * matZ.x + ( pos.y * matZ.y + ( pos.z * matZ.z + matZ.w ) ) ) );
}
vec4 MatrixMul ( mat4x4 m, vec4 v ) {
	return m * v;
}
vec4 MatrixMul ( vec4 v, mat4x4 m ) {
	return v * m;
}
vec3 MatrixMul ( mat3x3 m, vec3 v ) {
	return m * v;
}
vec3 MatrixMul ( vec3 v, mat3x3 m ) {
	return v * m;
}
vec2 MatrixMul ( mat2x2 m, vec2 v ) {
	return m * v;
}
vec2 MatrixMul ( vec2 v, mat2x2 m ) {
	return v * m;
}
vec2 screenPosToTexcoord ( vec2 pos, vec4 bias_scale ) { return ( pos * bias_scale.zw + bias_scale.xy ); }
vec2 screenPosToTexcoord ( vec2 pos, vec4 bias_scale, vec4 resolution_scale ) { return ( ( pos * bias_scale.zw + bias_scale.xy ) * resolution_scale.xy ); }
float GetLinearDepth ( float ndcZ, vec4 projectionMatrixZ, float rcpFarZ, bool bFirstPersonArmsRescale ) {
	float linearZ = projectionMatrixZ.w / ( ndcZ + projectionMatrixZ.z );
	if ( bFirstPersonArmsRescale ) {
		linearZ *= linearZ < 1.0 ? 10.0 : 1.0;
	}
	return linearZ * rcpFarZ;
}
float GetNdcDepth ( float linearZ, vec4 projectionMatrixZ ) {
	float ndcZ = ( projectionMatrixZ.w / linearZ ) - projectionMatrixZ.z;
	return ndcZ;
}
vec2 GetWindowPos ( vec3 viewPos, vec4 projection ) {
	return vec2( 0.5 ) + projection.xy * ( viewPos.xy / viewPos.z );
}
vec3 GetViewPos ( vec3 winPos, vec4 inverseProjection0, vec4 inverseProjection1 ) {
	return vec3( inverseProjection0.xy * winPos.xy + inverseProjection0.zw, inverseProjection1.z ) / ( inverseProjection1.x * winPos.z + inverseProjection1.y );
}
uint ReverseBits4 ( uint idx ) {
	idx = ( ( idx & 0x55555555 ) << 1) | ( ( idx & 0xAAAAAAAA ) >> 1 );
	idx = ( ( idx & 0x33333333 ) << 2) | ( ( idx & 0xCCCCCCCC ) >> 2 );
	return idx;
}
vec2 Hammersley2D_4bits ( uint numSamples, uint idx ) {
	const float rcpNumSamples = 1.0 / float( numSamples );
	return vec2( float( idx + 1 ) * rcpNumSamples, float( ReverseBits4( idx + 1 ) ) / 16.0 );
}
mat3x3 BuildBasis ( vec3 n ) {
	vec3 t = (n.x > 0.9 )? vec3( 0.0 , 1.0, 0.0 ) : vec3 ( 1.0, 0.0, 0.0 );
	t -= n * dot ( t , n );
	t = normalize( t );
	return mat3x3( t, cross ( n , t ), n );
}
vec2 OctWrap ( vec2 v ) {
	return ( 1.0 - abs( v.yx ) ) * vec2( ( v.x >= 0.0 ? 1.0 : -1.0 ), ( v.y >= 0.0 ? 1.0 : -1.0 ) );
}
vec3 NormalOctDecode ( vec2 encN, bool expand_range ) {
	if ( expand_range ) {
		encN = encN * 2.0 - 1.0;
	}
	vec3 n;
	n.z = 1.0 - abs( encN.x ) - abs( encN.y );
	n.xy = n.z >= 0.0 ? encN.xy : OctWrap( encN.xy );
	n = normalize( n );
	return n;
}
vec2 SmoothnessDecode ( float s ) {
	const float expanded_s = s * 2.0 - 1.0;
	return vec2( sqr( expanded_s ), expanded_s > 0 ? 1.0 : 0.0 );
}
void main() {
	{
		{
			vec2 tc = screenPosToTexcoord( gl_FragCoord.xy.xy, _fa_freqHigh[0 ] );
			{
				{
					bool taaEnabled = _fa_freqHigh[1 ].x == 1.0 ? true : false;
					int numRays = 12;
					int numRaysOuter = 8;
					float accNormalization = 1.0 / float( numRays - numRaysOuter );
					float accOuterNormalization = 1.0 / float( numRaysOuter );
					float angle_bias = _fa_freqHigh[2 ].z;
					float position_bias = _fa_freqHigh[2 ].w;
					out_FragColor0 = vec4( 1 );
					float ndcZ = tex2Dlod( samp_viewdepthmap, vec4( tc.xy, 0, 0 ) ).x;
					float linearZ = GetLinearDepth( ndcZ, _fa_freqLow[0 ], 1.0, false );
					float distAtten = saturate( ( _fa_freqHigh[1 ].y - linearZ ) / 256.0 );
					vec4 n1 = vec4( NormalOctDecode( tex2Dlod( samp_tex0, vec4( tc.xy, 0, 0 ) ).xy, false ).xyz, SmoothnessDecode( tex2Dlod( samp_tex3, vec4( tc.xy, 0, 0 ) ).w ).y ); if ( distAtten * n1.w > 0.0 ) {
						float firstPersonArmsZ = GetNdcDepth( 3.5, _fa_freqLow[0 ] );
						float radii = ( ndcZ < firstPersonArmsZ ) ? _fa_freqHigh[2 ].w : _fa_freqHigh[2 ].y;
						float radii_inner = ( ndcZ < firstPersonArmsZ ) ? _fa_freqHigh[2 ].z : _fa_freqHigh[2 ].x;
						float max_radius_ws = 1.0 / radii;
						float normalBias = ( ndcZ < firstPersonArmsZ )? _fa_freqHigh[1 ].w * 0.5 : _fa_freqHigh[1 ].w;
						ndcZ = ( ndcZ < firstPersonArmsZ ) ? ndcZ * 10 : ndcZ;
						vec3 normalM;
						vec3 n2 = NormalOctDecode( tex2Dlod( samp_tex0, vec4( tc.xy + 0.5 * _fa_freqHigh[3 ].zw * vec2( 1.0, 0.0 ), 0, 0 ) ).xy, false );
						vec3 n3 = NormalOctDecode( tex2Dlod( samp_tex0, vec4( tc.xy + 0.5 * _fa_freqHigh[3 ].zw * vec2( 0.0, 1.0 ), 0, 0 ) ).xy, false );
						vec3 n4 = NormalOctDecode( tex2Dlod( samp_tex0, vec4( tc.xy + 0.5 * _fa_freqHigh[3 ].zw * vec2( 1.0, 1.0 ), 0, 0 ) ).xy, false );
						vec3 gbufN = ( n1.xyz + n2 + n3 + n4 ) / 4.0 ;
						normalM = gbufN.xyz;
						mat3x3 invProjT = mat3x3( _fa_freqLow[1 ].xyz, _fa_freqLow[2 ].xyz, _fa_freqLow[3 ].xyz );
						vec3 normalVS = normalize( MatrixMul( invProjT, normalM.xyz ) );
						mat3x3 basisN = BuildBasis( normalVS );
						ndcZ -= _fa_freqHigh[1 ].z;
						vec3 viewPosOrig = GetViewPos( vec3( tc.xy / _fa_freqLow[4 ].xy, ndcZ ), _fa_freqLow[5 ], _fa_freqLow[6 ] );
						viewPosOrig.xyz += normalVS.xyz * normalBias;
						ivec2 pix_idx = ivec2( ivec2( gl_FragCoord.xy.xy ).xy + ( 0*ivec2( _fa_freqLow[7 ].xy ) ) ) & 1;
						int idx_pix = pix_idx.x + pix_idx.y * 2;
						vec3 noiseVec = ( vec3( Hammersley2D_4bits( 4, ( idx_pix + int( taaEnabled ? _fa_freqHigh[4 ].x : 0 ) ) & 3 ) * 2.0 - 1.0, 0 ) );
						noiseVec = normalize( MatrixMul( basisN, noiseVec.xyz ) );
						vec3 T = normalize( noiseVec - normalVS * dot( noiseVec, normalVS ) );
						vec3 B = cross( normalVS, T );
						mat3x3 TBN = mat3x3( T, B, normalVS );
						float unoccl_ratio = 0; for (int t = 0; t < numRays; t++ ) {
							radii = ( t >= numRaysOuter )? radii_inner : radii;
							vec3 ray_dir = vec3( MatrixMul( TBN, sharedparmsuniformbuffer[ t ].xyz ) );
							vec3 viewPosHemisphere = viewPosOrig.xyz + radii * ray_dir;
							vec3 winPosTap;
							winPosTap.xy = GetWindowPos( viewPosHemisphere, _fa_freqLow[8 ] ); if ( t < numRaysOuter ) {
								winPosTap.z = 1 - tex2Dlod( samp_tex2, vec4( winPosTap.xy * _fa_freqLow[4 ].xy, 0, 0 ) ).x;
							} else {
								winPosTap.z = 1 - tex2Dlod( samp_tex1, vec4( winPosTap.xy * _fa_freqLow[4 ].xy, 0, 0 ) ).x;
							}
							vec3 viewPosTap = GetViewPos( winPosTap, _fa_freqLow[5 ], _fa_freqLow[6 ] );
							float tap_discard = abs( viewPosOrig.z - viewPosTap.z ) < radii ? 1: 0;
							unoccl_ratio += ( viewPosTap.z > viewPosHemisphere.z ? 1 : 0 ) * tap_discard;
						}
						unoccl_ratio /= float( 12.0 );
						out_FragColor0.x = saturate( 1 - unoccl_ratio * distAtten );
					}
				}
			};;
		};
	}
}