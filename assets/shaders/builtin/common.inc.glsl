#define	IMU_DIFFUSE 0
#define	IMU_NORMAL 1
#define IMU_AORM 2
#define IMU_EMMISIVE 3
#define IMU_DEPTH 4
#define IMU_FRAGPOS 5
#define IMU_HDR 6
#define IMU_DEFAULT 7

const float PI = 3.14159265359;
const float ONE_OVER_PI = 1.0/PI;

#define FLG_X_COVERAGE_SHIFT 0
#define FLG_X_COVERAGE_MASK 3
#define FLG_X_COVERAGE_SOLID 0
#define FLG_X_COVERAGE_MASKED 1
#define FLG_X_COVERAGE_BLEND 2

float asfloat ( uint x ) { return uintBitsToFloat( x ); }
uint asuint ( float x ) { return floatBitsToUint( x ); }

float saturate(float x) { return clamp(x, 0.0, 1.0); }
vec2 saturate(vec2 x) { return clamp(x, 0.0, 1.0); }
vec3 saturate(vec3 x) { return clamp(x, 0.0, 1.0); }
vec4 saturate(vec4 x) { return clamp(x, 0.0, 1.0); }

vec3 tonemap(vec3 c) { return c / ( c + vec3(1.0) ); }
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
