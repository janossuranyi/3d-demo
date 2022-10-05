#version 330 core

out vec4 FS_OUT;
in vec3 vso_TexCoords;
uniform samplerCube samp0;

void main()
{   
    vec4 c = texture(samp0, vso_TexCoords);
    //float dp = fract( dot( gl_FragCoord.xy, vec2(0.5, 0.5) ) );
    //FS_OUT = mix(c, vec4(0.05, 0.01, 0.01, 1.0), dp < 0.5);

    FS_OUT = c;
}