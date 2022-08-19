#version 430 core

in vec2 vso_TexCoord;
out vec4 FS_OUT;

uniform sampler2D samp0;

void main()
{
	FS_OUT = texture(samp0, vso_TexCoord);
}