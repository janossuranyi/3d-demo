#include "version.inc.glsl"

out vec4 FS_OUT;
uniform samplerCube samp0;
uniform samplerBuffer g_vData;

in INTERFACE {
    vec4 TexCoords;
} In;

void main()
{   
    float dp = fract( dot( gl_FragCoord.xy, vec2(0.5, 0.5) ) );

    //FS_OUT = mix(vec4(c * In.vdata.xyz, 1.0), vec4(0.0, 0.0, 0.0, 1), dp < 0.5);
    //if(dp<0.5) discard;
    
    vec4 mod = texelFetch(g_vData, 220);
    vec3 c = texture(samp0, In.TexCoords.xyz).rgb;
    FS_OUT = mod * vec4(c,1.0);
}