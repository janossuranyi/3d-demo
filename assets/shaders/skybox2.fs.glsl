#include "version.inc.glsl"

out vec4 FS_OUT;
uniform samplerCube samp0;

in INTERFACE {
    vec4 TexCoords;
    flat uint flags;
} In;

void main()
{   
    vec3 c = texture(samp0, In.TexCoords.xyz).rgb;
    FS_OUT = vec4(c, 1.0);

/*
    const vec4 c0 = vec4(0,0,1,1);
    vec4 c = texture(samp0, In.TexCoords.xyz);

    bool flag_2 = (uint(In.flags) >> 1) == 1;
    float dp = fract( dot( gl_FragCoord.xy, vec2(0.5, 0.5) ) );
   
    FS_OUT = flag_2 ? mix(c, c0, dp < 0.5) : c;
*/
}