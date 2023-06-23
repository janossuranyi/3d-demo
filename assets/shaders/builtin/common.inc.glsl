
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

vec4 GammaIEC(vec4 c)
{
    return vec4(
        GammaIEC(c.r),
        GammaIEC(c.g),
        GammaIEC(c.b),
		GammaIEC(c.a)
	);
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
  return (result);
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

// Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
vec3 tonemap_aces(vec3 x) {
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// Unreal 3, Documentation: "Color Grading"
// Adapted to be close to Tonemap_ACES, with similar range
// Gamma 2.2 correction is baked in, don't use with sRGB conversion!
vec3 tonemap_unreal(vec3 x) {
  return x / (x + 0.155) * 1.019;
}

vec3 tonemap_reinhard2(vec3 x) {
  const float L_white = 4.0;

  return (x * (1.0 + x / (L_white * L_white))) / (1.0 + x);
}

// Lottes 2016, "Advanced Techniques and Optimization of HDR Color Pipelines"
vec3 tonemap_lottes(vec3 x) {
  const vec3 a = vec3(1.6);
  const vec3 d = vec3(0.977);
  const vec3 hdrMax = vec3(8.0);
  const vec3 midIn = vec3(0.18);
  const vec3 midOut = vec3(0.267);

  const vec3 b =
      (-pow(midIn, a) + pow(hdrMax, a) * midOut) /
      ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
  const vec3 c =
      (pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) /
      ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

  return pow(x, a) / (pow(x, a * d) * b + c);
}

// Uchimura 2017, "HDR theory and practice"
// Math: https://www.desmos.com/calculator/gslcdxvipg
// Source: https://www.slideshare.net/nikuque/hdr-theory-and-practicce-jp
vec3 uchimura(vec3 x, float P, float a, float m, float l, float c, float b) {
  float l0 = ((P - m) * l) / a;
  float L0 = m - m / a;
  float L1 = m + (1.0 - m) / a;
  float S0 = m + l0;
  float S1 = m + a * l0;
  float C2 = (a * P) / (P - S1);
  float CP = -C2 / P;

  vec3 w0 = vec3(1.0 - smoothstep(0.0, m, x));
  vec3 w2 = vec3(step(m + l0, x));
  vec3 w1 = vec3(1.0 - w0 - w2);

  vec3 T = vec3(m * pow(x / m, vec3(c)) + b);
  vec3 S = vec3(P - (P - S1) * exp(CP * (x - S0)));
  vec3 L = vec3(m + a * (x - m));

  return T * w0 + L * w1 + S * w2;
}

vec3 tonemap_uchimura(vec3 x) {
  const float P = 1.0;  // max display brightness
  const float a = 1.0;  // contrast
  const float m = 0.22; // linear section start
  const float l = 0.4;  // linear section length
  const float c = 1.33; // black
  const float b = 0.0;  // pedestal

  return uchimura(x, P, a, m, l, c, b);
}

vec2 screenPosToTexcoord(vec2 pos, vec4 scale) { return pos * scale.zw; }


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

vec2 NormalOctEncode ( vec3 n, bool compress_range ) {
	n /= ( abs( n.x ) + abs( n.y ) + abs( n.z ) );
	n.xy = n.z >= 0.0 ? n.xy : OctWrap( n.xy );
	if ( compress_range ) {
		n.xy = n.xy * 0.5 + 0.5;
	}
	return n.xy;
}

/*
			float ShoStren = _fa_freqHigh[31 ].x, LinStren = _fa_freqHigh[31 ].y, LinAngle = 0.1, ToeStren = 0.2, ToeNum = _fa_freqHigh[31 ].z, ToeDenom = 0.3;
			vec3 c = max( linearHDR.xyz, vec3( 0.0 ) );
			vec3 toneMapped = ( ( c * ( ShoStren * c + _fa_freqHigh[32 ].x ) + _fa_freqHigh[32 ].y ) / ( c * ( ShoStren * c + LinStren ) + ToeStren * ToeDenom ) ) - _fa_freqHigh[31 ].w;
*/