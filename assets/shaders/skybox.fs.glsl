#version 330 core

out vec4 FS_OUT;
in vec3 vso_TexCoords;
uniform samplerCube samp0;

void main()
{    
    FS_OUT = texture(samp0, vso_TexCoords);
}