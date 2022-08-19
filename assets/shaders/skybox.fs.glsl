#version 330 core

out vec4 FS_OUT;
in vec3 vso_TexCoords;
uniform samplerCube samp0;

vec4 pow4(vec4 v0, float p)
{
    return vec4(
        pow(v0.r, p),
        pow(v0.g, p),
        pow(v0.b, p),
        v0.a);
}

void main()
{    
    FS_OUT = texture(samp0, vso_TexCoords);
}