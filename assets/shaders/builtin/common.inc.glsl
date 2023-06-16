
const float PI = 3.14159265359;
const float ONE_OVER_PI = 1.0/PI;

float asfloat ( uint x ) { return uintBitsToFloat( x ); }
uint asuint ( float x ) { return floatBitsToUint( x ); }

float saturate(float x) { return clamp(x, 0.0, 1.0); }
vec2 saturate(vec2 x) { return clamp(x, 0.0, 1.0); }
vec3 saturate(vec3 x) { return clamp(x, 0.0, 1.0); }
vec4 saturate(vec4 x) { return clamp(x, 0.0, 1.0); }

float GammaIEC(float c) { return c <= 0.0031308 ? c * 12.92 : 1.055 * pow(c, 1/2.4) -0.055; }
vec3 GammaIEC(vec3 c)
{
    return vec3(
        GammaIEC(c.r),
        GammaIEC(c.g),
        GammaIEC(c.b));
}
float SRGBlinear ( float value ) {
	if ( value <= 0.04045 ) {
		return ( value / 12.92 );
	} else {
		return pow( ( value / 1.055 ) + 0.0521327, 2.4 );
	}
}
vec3 SRGBlinear ( vec3 sRGB ) {
	vec3 outLinear;
	outLinear.r = SRGBlinear( sRGB.r );
	outLinear.g = SRGBlinear( sRGB.g );
	outLinear.b = SRGBlinear( sRGB.b );
	return outLinear;
}

vec4 SRGBlinear ( vec4 sRGBA ) { return vec4( SRGBlinear( sRGBA.rgb ), sRGBA.a );}

// Filmic Tonemapping Operators http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 tonemap_filmic(vec3 x) {
  vec3 X = max(vec3(0.0), x - 0.004);
  vec3 result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);
  return SRGBlinear(result);
}

vec3 tonemap_Reinhard(vec3 c) { return c / ( c + vec3(1.0) ); }
vec3 tonemap_Exp(vec3 c) { return vec3(1.0) - exp(-c); }

vec3 tonemap_Uncharted2(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;

	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

/*
			float ShoStren = _fa_freqHigh[31 ].x, LinStren = _fa_freqHigh[31 ].y, LinAngle = 0.1, ToeStren = 0.2, ToeNum = _fa_freqHigh[31 ].z, ToeDenom = 0.3;
			vec3 c = max( linearHDR.xyz, vec3( 0.0 ) );
			vec3 toneMapped = ( ( c * ( ShoStren * c + _fa_freqHigh[32 ].x ) + _fa_freqHigh[32 ].y ) / ( c * ( ShoStren * c + LinStren ) + ToeStren * ToeDenom ) ) - _fa_freqHigh[31 ].w;
*/