#include "version.inc.glsl"

out vec4 FS_OUT;
uniform samplerCube samp0;
uniform samplerBuffer g_vData;

in INTERFACE {
    vec4 TexCoords;
} In;

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
vec4 SRGBlinear ( vec4 sRGBA ) {
	vec4 outLinear = vec4( SRGBlinear( sRGBA.rgb ), 1 );
	outLinear.a = SRGBlinear( sRGBA.a );
	return outLinear;
}

void main()
{   
    float dp = fract( dot( gl_FragCoord.xy, vec2(0.5, 0.5) ) );

    //FS_OUT = mix(vec4(c * In.vdata.xyz, 1.0), vec4(0.0, 0.0, 0.0, 1), dp < 0.5);
    //if(dp<0.5) discard;
    
    vec4 mod = texelFetch(g_vData, 220);
    vec3 c = ( texture(samp0, In.TexCoords.xyz).rgb );
    FS_OUT = mod * vec4(c,1.0);
}