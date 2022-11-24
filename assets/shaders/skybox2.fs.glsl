#include "version.inc.glsl"

out vec4 FS_OUT;
uniform samplerCube samp0;

in INTERFACE {
    vec4 TexCoords;
    flat uint flags;
    flat vec4 vdata;
} In;

void main()
{   
    vec3 c = texture(samp0, In.TexCoords.xyz).rgb;
    float dp = fract( dot( gl_FragCoord.xy, vec2(0.5, 0.5) ) );

    FS_OUT = mix(vec4(c * In.vdata.xyz, 1.0), vec4(0.05, 0.1, 0.1, 1), dp < 0.5);
}