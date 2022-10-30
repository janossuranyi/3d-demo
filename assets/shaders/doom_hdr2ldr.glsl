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

uniform vec4 _fa_freqHigh [33];
uniform vec4 _fa_freqLow [7];
uniform sampler2D samp_postdistortionmap;
uniform sampler2D samp_tex0; // HDRI source
uniform sampler2D samp_tex1; // Bloom
uniform sampler2D samp_tex3;
uniform sampler2D samp_bloomdustmap;
uniform sampler2D samp_lensvignettingmap;
uniform sampler2D samp_flowvariation;
uniform sampler2D samp_poweruppulse;
uniform sampler2D samp_screenoverlaytex2;
uniform sampler2D samp_screenoverlaytex1;
uniform sampler2D samp_emdistortionmap;
uniform sampler2D samp_outlinesmap;
uniform sampler2D samp_viewdepthmap;
uniform sampler2D samp_cacodemoncolormap;
uniform sampler2D samp_cacodemondistortionmap;

in vec4 vofi_TexCoord0;
in vec4 gl_FragCoord;

out vec4 out_FragColor0;
out vec4 out_FragColor1;

float linearSRGB ( float value ) { if ( value > 0.0031308 ) {
		return pow( value, 1.0 / 2.4 ) * 1.055 - 0.055;
	} else {
		return value * 12.92;
	}
}
vec3 linearSRGB ( vec3 inLinear ) {
	vec3 sRGB;
	sRGB.r = linearSRGB( inLinear.r );
	sRGB.g = linearSRGB( inLinear.g );
	sRGB.b = linearSRGB( inLinear.b );
	return sRGB;
}
vec4 linearSRGB ( vec4 inLinear ) {
	vec4 sRGBA = vec4( linearSRGB( inLinear.rgb ), 1 );
	sRGBA.a = linearSRGB( inLinear.a );
	return sRGBA;
}
float SRGBlinearApprox ( float value ) {
	return value * ( value * ( value * 0.305306011 + 0.682171111 ) + 0.012522878 );
}
vec3 SRGBlinearApprox ( vec3 sRGB ) {
	vec3 outLinear;
	outLinear.r = SRGBlinearApprox( sRGB.r );
	outLinear.g = SRGBlinearApprox( sRGB.g );
	outLinear.b = SRGBlinearApprox( sRGB.b );
	return outLinear;
}
vec4 SRGBlinearApprox ( vec4 sRGBA ) {
	vec4 outLinear = vec4( SRGBlinearApprox( sRGBA.rgb ), 1 );
	outLinear.a = SRGBlinearApprox( sRGBA.a );
	return outLinear;
}
vec4 sqr ( vec4 x ) { return ( x * x ); }
vec3 sqr ( vec3 x ) { return ( x * x ); }
vec2 sqr ( vec2 x ) { return ( x * x ); }
float sqr ( float x ) { return ( x * x ); }
vec2 screenPosToTexcoord ( vec2 pos, vec4 bias_scale ) { return ( pos * bias_scale.zw + bias_scale.xy ); }
vec2 screenPosToTexcoord ( vec2 pos, vec4 bias_scale, vec4 resolution_scale ) { return ( ( pos * bias_scale.zw + bias_scale.xy ) * resolution_scale.xy ); }
vec4 tex2DlodClamped ( sampler2D image, vec4 texcoord, vec4 scalingInfos ) {
	vec2 pixelSize = scalingInfos.zw;
	vec2 clampedTex = clamp( texcoord.xy, vec2( 0 ), scalingInfos.xy - pixelSize / 2. );
	return tex2Dlod( image, vec4( clampedTex, texcoord.zw ) );
}
float GetLuma ( vec3 c ) {
	return dot( c, vec3( 0.2126, 0.7152, 0.0722 ) );
}
void main() {
	{
		float autoExposure = vofi_TexCoord0.x;
		vec2 texcoord = screenPosToTexcoord( gl_FragCoord.xy, _fa_freqHigh[0 ] );
		vec3 outputColor;
		{
			vec2 unscaledTexcoord = texcoord.xy / _fa_freqLow[0 ].xy;
			{
				vec2 distortion = tex2Dlod( samp_postdistortionmap, vec4( unscaledTexcoord.xy, 0.0, 0.0 ) ).xy - vec2( 127.0 / 255.0 );
				texcoord.xy += distortion.xy * 0.1;
			}
			vec2 texcoordD = ( unscaledTexcoord.xy - vec2( 0.5, 0.5 ) );
			float directionLen = length( texcoordD );
			texcoordD *= _fa_freqHigh[1 ].x * pow( directionLen, _fa_freqHigh[1 ].y ) / directionLen * _fa_freqLow[0 ].xy;
			vec2 texcoordR2 = texcoord.xy - texcoordD;
			vec2 texcoordR = texcoord.xy - texcoordD * 0.5;
			vec2 texcoordG = texcoord.xy;
			vec2 texcoordB = texcoord.xy + texcoordD * 0.5;
			vec2 texcoordB2 = texcoord.xy + texcoordD;
			vec4 resScalingInfo = vec4( _fa_freqLow[0 ].xy, _fa_freqHigh[0 ].zw );
			vec3 linearHDR =
			vec3( 0.5, 0.0, 0.0 ) * tex2DlodClamped( samp_tex0, vec4( texcoordR2.xy, 0.0, 0.0 ), resScalingInfo ).xyz +
			vec3( 0.3, 0.25, 0.0 ) * tex2DlodClamped( samp_tex0, vec4( texcoordR.xy, 0.0, 0.0 ), resScalingInfo ).xyz +
			vec3( 0.2, 0.5, 0.2 ) * tex2DlodClamped( samp_tex0, vec4( texcoordG.xy, 0.0, 0.0 ), resScalingInfo ).xyz +
			vec3( 0.0, 0.25, 0.3 ) * tex2DlodClamped( samp_tex0, vec4( texcoordB.xy, 0.0, 0.0 ), resScalingInfo ).xyz +
			vec3( 0.0, 0.0, 0.5 ) * tex2DlodClamped( samp_tex0, vec4( texcoordB2.xy, 0.0, 0.0 ), resScalingInfo ).xyz;
			vec3 bloom = tex2Dlod( samp_tex1, vec4( unscaledTexcoord, 0.0, 0.0 ) ).xyz * _fa_freqHigh[2 ].xyz;
			vec3 flares = tex2Dlod( samp_tex3, vec4( texcoord.xy, 0, 0 ) ).xyz * _fa_freqHigh[3 ].z;
			float vignetting = 1.0 - pow( directionLen, _fa_freqHigh[1 ].w ) / directionLen * _fa_freqHigh[1 ].z;
			vec3 lensDirt = tex2Dlod( samp_bloomdustmap, vec4( 1.4 * ( texcoord.xy / _fa_freqLow[0 ].xy ) * vec2( 1.0, _fa_freqHigh[4 ].z ), 0.0, 0 ) ).rgb; if ( _fa_freqLow[1 ].x == 2 ) {
				vec4 l1_ck = vec4( 0.034142252, 0.059078815, 0.071750865, 0.9 );
				vec4 l2_ck = vec4( 0.049826989, 0.094585159, 0.097660899, 0.85 );
				vec2 tc1 = ( unscaledTexcoord - 0.5 ) * - 0.1 + 0.5;
				vec2 tc2 = ( unscaledTexcoord - 0.5 ) * - 0.3 + 0.5;
				vec3 lensVignetting = sqr( tex2Dlod( samp_lensvignettingmap, vec4( ( unscaledTexcoord - 0.5 ) *0.8 + 0.5, 0, 0 ) ).xyz );
				lensVignetting = sqr( tex2Dlod( samp_lensvignettingmap, vec4( ( unscaledTexcoord - 0.5 ) *0.8 + 0.5, 0, 3 ) ).xyz ) * ( 1 - lensVignetting );
				vec3 lensRefl = lensVignetting * mix( tex2Dlod( samp_tex0, vec4( tc1, 0, 0 ) ).xyz, tex2Dlod( samp_tex1, vec4( tc1, 0, 0 ) ).xyz, l1_ck.www ) * l1_ck.xyz * 0.75;
				lensRefl += lensVignetting * mix( tex2Dlod( samp_tex0, vec4( tc2, 0, 0 ) ).xyz, tex2Dlod( samp_tex1, vec4( tc2, 0, 0 ) ).xyz, l2_ck.www ) * l2_ck.xyz * 0.75;
				bloom += lensRefl * _fa_freqHigh[2 ].xyz * _fa_freqHigh[3 ].z;
			}
			linearHDR.xyz += flares + ( ( flares + bloom * lensDirt ) ) * _fa_freqHigh[4 ].w;
			linearHDR.xyz = autoExposure * ( linearHDR.xyz * _fa_freqHigh[4 ].x + bloom.xyz * _fa_freqHigh[4 ].y );
			linearHDR.xyz = _fa_freqLow[1 ].x == 0.0 ? mix( linearHDR.xyz, _fa_freqHigh[5 ].xyz, 1.0 - vignetting ) : linearHDR.xyz * vignetting;
			uvec2 idxPix = uvec2( gl_FragCoord.xy ) & 3;
			uvec2 bayerMat = uvec2( ( 0 << 0 ) | ( 12 << 4 ) | ( 3 << 8 ) | ( 15 << 12 ) | ( 8 << 16 ) | ( 4 << 20 ) | ( 11 << 24 ) | ( 7 << 28 ),
			( 2 << 0 ) | ( 14 << 4 ) | ( 1 << 8 ) | ( 13 << 12 ) | ( 10 << 16 ) | ( 6 << 20 ) | ( 9 << 24 ) | ( 5 << 28 ) );
			float dither = float( ( ( ( bayerMat[ idxPix.y >> 1 ] >> ( ( idxPix.x + ( idxPix.y & 1 ) * 4 ) * 4 ) ) & 0xF ) ) + 1 ) / 4335.0;
			{
				vec2 unscaledTC = texcoord.xy / _fa_freqLow[0 ].xy; if ( _fa_freqHigh[6 ].w != 0.0 ) {
					float pulse = sin( _fa_freqLow[2 ].x * 5.0 ) * 0.1 + 0.9;
					float mask = unscaledTC.x * ( 1.0 - unscaledTC.x ) * unscaledTC.y * ( 1.0 - unscaledTC.y );
					mask = saturate( 1.0 - mask );
					mask = pow(mask, 20.0);
					mask = 1.0 - mask;
					vec4 flowMap = tex2Dlod( samp_flowvariation, vec4( unscaledTexcoord.xy * vec2( _fa_freqHigh[7 ].x, _fa_freqHigh[7 ].y ) + vec2( _fa_freqLow[2 ].x * _fa_freqHigh[7 ].z, _fa_freqLow[2 ].x * _fa_freqHigh[7 ].w ), 0, 0 ) );
					flowMap.xy = flowMap.xy * 2.0 - 1.0;
					vec2 uvFlowDirection = vec2( flowMap.x * 0.25, flowMap.y * 0.55 );
					flowMap.xyzw = vec4( unscaledTC.xy, unscaledTC.xy ) + vec4( uvFlowDirection, uvFlowDirection );
					flowMap.xy = mix( unscaledTC.xy, flowMap.xy, vec2( mask ) );
					vec4 powerUpPulse = tex2Dlod( samp_poweruppulse, vec4( flowMap.xy, 0, 0 ) );
					vec4 powerUpPulseStatic = tex2Dlod( samp_poweruppulse, vec4( unscaledTC, 0, 0 ) );
					powerUpPulseStatic = SRGBlinearApprox( powerUpPulseStatic );
					powerUpPulse = SRGBlinearApprox( powerUpPulse );
					powerUpPulse = mix( powerUpPulse, pow( powerUpPulse, vec4( 5.0 ) ), _fa_freqHigh[8 ].x );
					powerUpPulse.xyz = powerUpPulse.xyz * powerUpPulseStatic.xyz;
					vec3 inColor = linearHDR.xyz;
					linearHDR.xyz = mix( linearHDR.xyz, linearHDR.xyz + powerUpPulse.xyz * _fa_freqHigh[6 ].xyz * pulse, powerUpPulse.w );
					linearHDR.xyz = mix( inColor, linearHDR.xyz, _fa_freqHigh[6 ].w );
				}
				{
					float overlayWeight = _fa_freqHigh[9 ].z; if ( overlayWeight > 0.0 ) {
						bool bottomOnly = _fa_freqHigh[9 ].w > 0.0;
						vec2 dimensions = vec2( _fa_freqHigh[9 ].x, _fa_freqHigh[9 ].y );
						bvec2 mirrorTiles = bvec2( dimensions.x >= 0.0, dimensions.y >= 0.0 );
						dimensions = abs( dimensions );
						bvec2 tiled = bvec2( dimensions.x <= 0.5, dimensions.y <= 0.5 );
						dimensions = min( dimensions, vec2( 0.5 ) );
						if ( abs( 0.5 - unscaledTC.x ) >= 0.5 - dimensions.x ) {
							if ( abs( 0.5 - unscaledTC.y ) >= 0.5 - dimensions.y && ( !bottomOnly || unscaledTC.y <= 0.5 ) ) {
								vec2 overlayTexcoord = unscaledTC.xy; if ( tiled.x ) {
									overlayTexcoord.x = unscaledTC.x <= 0.5 ? ( unscaledTC.x / dimensions.x ) : ( unscaledTC.x + dimensions.x - 1.0 ) / dimensions.x; if ( mirrorTiles.x ) {
										overlayTexcoord.x = unscaledTC.x <= 0.5 ? overlayTexcoord.x : 1.0 - overlayTexcoord.x;
									}
								} if ( tiled.y ) {
									overlayTexcoord.y = unscaledTC.y <= 0.5 ? ( unscaledTC.y / dimensions.y ) : ( unscaledTC.y + dimensions.y - 1.0 ) / dimensions.y; if ( mirrorTiles.y ) {
										overlayTexcoord.y = unscaledTC.y <= 0.5 ? overlayTexcoord.y : 1. - overlayTexcoord.y;
									}
								}
								vec4 overlayColor = tex2Dlod( samp_screenoverlaytex2, vec4( overlayTexcoord, 0, 0 ) );
								overlayColor *= _fa_freqHigh[10 ];
								linearHDR.xyz = mix( linearHDR.xyz, overlayColor.xyz, overlayColor.w * overlayWeight );
							}
						}
					}
				};
				{
					float overlayWeight = _fa_freqHigh[11 ].z; if ( overlayWeight > 0.0 ) {
						bool bottomOnly = _fa_freqHigh[11 ].w > 0.0;
						vec2 dimensions = vec2( _fa_freqHigh[11 ].x, _fa_freqHigh[11 ].y );
						bvec2 mirrorTiles = bvec2( dimensions.x >= 0.0, dimensions.y >= 0.0 );
						dimensions = abs( dimensions );
						bvec2 tiled = bvec2( dimensions.x <= 0.5, dimensions.y <= 0.5 );
						dimensions = min( dimensions, vec2( 0.5 ) );
						if ( abs( 0.5 - unscaledTC.x ) >= 0.5 - dimensions.x ) {
							if ( abs( 0.5 - unscaledTC.y ) >= 0.5 - dimensions.y && ( !bottomOnly || unscaledTC.y <= 0.5 ) ) {
								vec2 overlayTexcoord = unscaledTC.xy; if ( tiled.x ) {
									overlayTexcoord.x = unscaledTC.x <= 0.5 ? ( unscaledTC.x / dimensions.x ) : ( unscaledTC.x + dimensions.x - 1.0 ) / dimensions.x; if ( mirrorTiles.x ) {
										overlayTexcoord.x = unscaledTC.x <= 0.5 ? overlayTexcoord.x : 1.0 - overlayTexcoord.x;
									}
								} if ( tiled.y ) {
									overlayTexcoord.y = unscaledTC.y <= 0.5 ? ( unscaledTC.y / dimensions.y ) : ( unscaledTC.y + dimensions.y - 1.0 ) / dimensions.y; if ( mirrorTiles.y ) {
										overlayTexcoord.y = unscaledTC.y <= 0.5 ? overlayTexcoord.y : 1. - overlayTexcoord.y;
									}
								}
								vec4 overlayColor = tex2Dlod( samp_screenoverlaytex1, vec4( overlayTexcoord, 0, 0 ) );
								overlayColor *= _fa_freqHigh[12 ];
								linearHDR.xyz = mix( linearHDR.xyz, overlayColor.xyz, overlayColor.w * overlayWeight );
							}
						}
					}
				}; if ( _fa_freqHigh[13 ].x > 0.0 ) {
					vec4 scanlineTex = tex2Dlod( samp_emdistortionmap, vec4( texcoord.xy * vec2( 0.5, 100.0 ) + floor( vec2( _fa_freqLow[2 ].x * 15.0, _fa_freqLow[2 ].x * 20.0 ) ) * 0.3, 0, 0 ) );
					float colorBlend = linearHDR.x + ( 0.3333 * linearHDR.y ) + ( 0.6666 * linearHDR.z );
					linearHDR.xyz = mix( linearHDR.xyz, mix( vec3( colorBlend ) , vec3( colorBlend ) * vec3( 0.3, 0.6, 0.0 ) , scanlineTex.x ), saturate( _fa_freqHigh[13 ].y * 0.5 * ( 1.0 - colorBlend ) ) ); if ( _fa_freqHigh[13 ].x >= 2.0 ) {
						vec4 outlineTex = tex2Dlod( samp_outlinesmap, vec4( unscaledTC.xy, 0, 0 ) );
						float mask = saturate( dot( outlineTex.xyz, vec3( 1 ) ) );
						linearHDR.xyz = mix( linearHDR.xyz, outlineTex.xyz, mask );
					}
				} if ( _fa_freqHigh[14 ].x == 1.0 && _fa_freqHigh[13 ].x == 0.0) {
					float grayscale = ( 0.21 * linearHDR.x )+ (0.72 * linearHDR.y) + (0.07 * linearHDR.z);
					vec4 outlineTex = tex2Dlod( samp_outlinesmap, vec4(unscaledTC.xy, 0, 0) );
					vec3 infraRedWorld = vec3( grayscale );
					vec3 playerInitialOutline = vec3( outlineTex.x * 0.62, 0.01, 0.0 ) ;
					vec3 demonInitialOutline = vec3( outlineTex.y * .62, 0.13, 0.0 ) ;
					vec3 initialOutline = mix(playerInitialOutline, demonInitialOutline, outlineTex.y);
					vec3 outlines = mix( vec3( 0.0 ), initialOutline.xyz * (linearHDR.x * 60.5), _fa_freqHigh[15 ].x );
					float pingScalar;
					vec3 gridColor;
					{
						float gridPowerValue = 30.0;
						float gridSinScalar = 1.0 / 3.0;
						float gridNearFade = 600;
						float gridDistanceFadeValue = 1000;
						vec3 baseGridColor = vec3( 1.0, 0.32, 0.0 );
						vec4 globalPos;
						{
							vec2 texcoordGrid = screenPosToTexcoord( gl_FragCoord.xy, _fa_freqHigh[0 ] );
							vec2 scaledTCGrid = vec2( texcoordGrid.x / _fa_freqLow[0 ].x, texcoordGrid.y / _fa_freqLow[0 ].y );
							float ndcZ = tex2Dlod( samp_viewdepthmap, vec4( scaledTCGrid.xy, 0, 0 ) ).x;
							vec4 device = vec4( scaledTCGrid.x * 2.0 - 1.0, scaledTCGrid.y * 2.0 - 1.0, ndcZ, 1.0 );
							globalPos.x = dot( device, _fa_freqLow[3 ] );
							globalPos.y = dot( device, _fa_freqLow[4 ] );
							globalPos.z = dot( device, _fa_freqLow[5 ] );
							globalPos.w = dot( device, _fa_freqLow[6 ] );
							globalPos.xyzw /= vec4( globalPos.w );
						}
						float distance = length( globalPos.xyz - _fa_freqHigh[16 ].xyz );
						float maxPingRange = _fa_freqHigh[17 ].x;
						float minPingRange = max( 0.0, _fa_freqHigh[17 ].x - _fa_freqHigh[17 ].y );
						float maxVisionDistance = _fa_freqHigh[17 ].z;
						float pingPowerExponent = _fa_freqHigh[17 ].w;
						float gridVal;
						{
							vec3 grid = saturate( pow( sin( globalPos.xyz * gridSinScalar ), vec3( gridPowerValue, gridPowerValue, gridPowerValue ) ) );
							gridVal = max( max( grid.x, grid.y ), grid.z );
							gridVal = ( distance < gridNearFade ) ? ( gridVal * distance / gridNearFade ) : gridVal;
							float distanceFade = saturate( ( gridDistanceFadeValue * gridDistanceFadeValue ) / ( distance * distance ) );
							gridVal = mix( 0.1, gridVal, distanceFade );
						}
						float scalarAlongPing = ( distance - minPingRange ) / ( maxPingRange - minPingRange );
						pingScalar = saturate( (scalarAlongPing > 1.0) ? 0.0 : scalarAlongPing );
						pingScalar = pow( pingScalar, pingPowerExponent );
						pingScalar *= _fa_freqHigh[15 ].x;
						float scalarOfMaxDistance = distance / maxVisionDistance;
						float maxDistanceFalloffScalar = 1.0 - saturate( (scalarOfMaxDistance - 1.0) * 10.0 );
						pingScalar = saturate( pingScalar * maxDistanceFalloffScalar );
						gridColor = mix( linearHDR.xyz, baseGridColor, gridVal );
					}
					vec3 outlinedColor = mix( linearHDR.xyz, mix( infraRedWorld, outlines.xyz, saturate( outlines.x * 2.0 ) ), _fa_freqHigh[18 ].x );
					linearHDR.xyz = mix( outlinedColor.xyz, gridColor, pingScalar );
				} if ( _fa_freqHigh[14 ].x == 1.0 && _fa_freqHigh[13 ].x == 1.0 ) {
					vec4 outlineTex = tex2Dlod( samp_outlinesmap, vec4(unscaledTC.xy, 0, 0) );
					vec3 playerInitialOutline = vec3( outlineTex.x * 0.62, 0.01, 0.62 ) ;
					vec3 playerTargetedInitialOutline = vec3( 0.62, 0.03, 0.0 ) ;
					vec3 initialOutline = mix(playerInitialOutline, playerTargetedInitialOutline, outlineTex.y);
					vec3 outlines = mix( vec3( 0.0 ), initialOutline.xyz * (linearHDR.x * 60.5 ), _fa_freqHigh[15 ].x );
					float noiseValue;
					vec4 waveColor;
					float waveVal;
					{
						vec4 globalPos;
						{
							vec2 texcoordGrid = screenPosToTexcoord( gl_FragCoord.xy, _fa_freqHigh[0 ] );
							vec2 scaledTCGrid = vec2( texcoordGrid.x / _fa_freqLow[0 ].x, texcoordGrid.y / _fa_freqLow[0 ].y );
							float ndcZ = tex2Dlod( samp_viewdepthmap, vec4( scaledTCGrid.xy, 0, 0 ) ).x;
							vec4 device = vec4( scaledTCGrid.x * 2.0 - 1.0, scaledTCGrid.y * 2.0 - 1.0, ndcZ, 1.0 );
							globalPos.x = dot( device, _fa_freqLow[3 ] );
							globalPos.y = dot( device, _fa_freqLow[4 ] );
							globalPos.z = dot( device, _fa_freqLow[5 ] );
							globalPos.w = dot( device, _fa_freqLow[6 ] );
							globalPos.xyzw /= vec4( globalPos.w );
						}
						float distance = length( globalPos.xyz - _fa_freqHigh[16 ].xyz );
						float maxPingRange = _fa_freqHigh[17 ].x;
						float minPingRange = _fa_freqHigh[17 ].x - _fa_freqHigh[17 ].y;
						float maxVisionDistance = _fa_freqHigh[17 ].z;
						float maxDistance = min( maxPingRange, maxVisionDistance );
						{
							float waveNearFade = 600;
							float waveMinDist = 50;
							waveVal = distance > maxDistance || distance <= minPingRange ? 0.0 : 1.0;
							waveVal = step( waveMinDist, distance ) *
							( distance < ( waveNearFade + waveMinDist ) ? waveVal * ( ( distance - waveMinDist) / waveNearFade ) : waveVal );
						}
						float waveDepth = 1.0 - saturate( ( distance - minPingRange ) / ( maxPingRange - minPingRange ) );
						waveColor = tex2Dlod( samp_cacodemoncolormap, vec4( waveDepth, 0, 0, 0 ) );
						vec2 direction = globalPos.xy - _fa_freqHigh[16 ].xy;
						float radial = atan( direction.x, direction.y ) / 3.1415926;
						float depthNoiseLookup = mod( ( distance - ( maxPingRange / 4.0 ) ) / _fa_freqHigh[19 ].x, 1.0 );
						float heightNoiseLookup = mod( globalPos.z / _fa_freqHigh[19 ].y, 1.0 );
						float depthNoise = tex2Dlod( samp_cacodemondistortionmap, vec4( radial, depthNoiseLookup, 0, 0 ) ).x;
						float heightNoise = tex2Dlod( samp_cacodemondistortionmap, vec4( radial, heightNoiseLookup, 0, 0 ) ).y;
						noiseValue = saturate( ( 2.0 * ( depthNoise - 0.5 ) ) + ( 2.0 * ( heightNoise - 0.5 ) ) );
						waveColor.xyz *= waveVal;
					}
					vec3 outlinedColor = mix( linearHDR.xyz, outlines.xyz, saturate( outlines.x * 2.0 ) );
					float waveScale = waveColor.w * noiseValue;
					float pingPowerExponent = _fa_freqHigh[17 ].w;
					linearHDR.xyz = outlinedColor.xyz + waveColor.xyz * waveScale * pingPowerExponent;
				} if ( ( _fa_freqHigh[20 ].x > 0.0 ) && ( _fa_freqHigh[14 ].x == 0.0 ) && ( _fa_freqHigh[13 ].x == 0.0 ) ) {
					vec4 outlineTex = tex2Dlod( samp_outlinesmap, vec4( unscaledTC.xy, 0, 0 ) );
					linearHDR.xyz += outlineTex.xyz;
				} if ( _fa_freqHigh[21 ].x > 0.0 ) {
					vec4 emiTex = tex2Dlod( samp_emdistortionmap, vec4( texcoord.xy * vec2( 1.3, 1.3 ) + floor( vec2( _fa_freqLow[2 ].x * 15.0, _fa_freqLow[2 ].x * 20.0 ) ) * 0.3, 0, 0 ) );
					linearHDR.xyz = mix( linearHDR.xyz, emiTex.xyz, saturate( _fa_freqHigh[21 ].x ) ) * vec3( 1.0, 0.2, 0.1 );
				} if ( _fa_freqHigh[22 ].x > 0.0 ) {
					float eyeCrossX = mix( 1.0, 0.9, _fa_freqHigh[22 ].x );
					float eyeCrossOffset = mix( 0.0, 0.1, _fa_freqHigh[22 ].x );
					float eyeCrossOffsetR = eyeCrossOffset * 0.5;
					vec4 eyeR = tex2Dlod( samp_tex0, vec4( texcoord.xy * vec2( eyeCrossX, 1.0 ) + vec2( eyeCrossOffset, 0.0 ) - vec2( eyeCrossOffsetR, 0.0 ), vec2( 0.0 ) ) );
					vec4 eyeL = tex2Dlod( samp_tex0, vec4( texcoord.xy * vec2( eyeCrossX, 1.0 ) + vec2( eyeCrossOffset, 0.0 ), vec2( 0.0 ) ) );
					vec3 doubleVision = mix( eyeR.xyz, eyeL.xyz, vec3( 0.5 ) );
					linearHDR.xyz = mix( linearHDR.xyz, doubleVision, vec3( _fa_freqHigh[22 ].x ) );
				} if ( _fa_freqHigh[23 ].x > 0.0 ) {
					float desaturated = GetLuma( linearHDR.xyz );
					linearHDR.xyz = vec3( desaturated );
				} if ( _fa_freqHigh[24 ].w > 0.0 ) {
					linearHDR.xyz = mix( linearHDR.xyz, _fa_freqHigh[24 ].xyz, _fa_freqHigh[24 ].w );
				}
			};
			float lum = GetLuma( linearHDR.xyz );
			float shadow2Mid = saturate( lum * _fa_freqHigh[25 ].x + _fa_freqHigh[25 ].y );
			float mid2High = saturate( lum * _fa_freqHigh[25 ].z + _fa_freqHigh[25 ].w );
			float saturation = _fa_freqHigh[26 ].z * mid2High + _fa_freqHigh[26 ].y * shadow2Mid + _fa_freqHigh[26 ].x;
			float gamma = _fa_freqHigh[27 ].z * mid2High + _fa_freqHigh[27 ].y * shadow2Mid + _fa_freqHigh[27 ].x;
			vec4 color = _fa_freqHigh[28 ] * mid2High + _fa_freqHigh[29 ] * shadow2Mid + _fa_freqHigh[30 ];
			linearHDR.xyz = mix( vec3( lum ), linearHDR.xyz, saturation );
			linearHDR.xyz = linearHDR.xyz * color.xyz + vec3( color.w );
			float ShoStren = _fa_freqHigh[31 ].x, LinStren = _fa_freqHigh[31 ].y, LinAngle = 0.1, ToeStren = 0.2, ToeNum = _fa_freqHigh[31 ].z, ToeDenom = 0.3;
			vec3 c = max( linearHDR.xyz, vec3( 0.0 ) );
			vec3 toneMapped = ( ( c * ( ShoStren * c + _fa_freqHigh[32 ].x ) + _fa_freqHigh[32 ].y ) / ( c * ( ShoStren * c + LinStren ) + ToeStren * ToeDenom ) ) - _fa_freqHigh[31 ].w;
			outputColor = saturate( linearSRGB( pow( toneMapped, vec3( gamma ) ) * _fa_freqHigh[32 ].z ) + dither );
			outputColor = saturate( mix( vec3( 0.5 ), outputColor.xyz, _fa_freqHigh[3 ].www ) );
			if ( _fa_freqHigh[3 ].x > 2.0 ) {
				outputColor.x = saturate( 1.0 - shadow2Mid );
				outputColor.y = saturate( shadow2Mid - mid2High );
				outputColor.z = mid2High;
			}
		};
		out_FragColor0.xyz = outputColor;
		out_FragColor1.xyz = outputColor;
	}
}