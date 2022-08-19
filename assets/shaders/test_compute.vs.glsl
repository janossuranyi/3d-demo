#version 430 core

layout(location = 0) in vec2 vaPosition;
layout(location = 1) in vec2 vaTexCoord;

out vec2 vso_TexCoord;

void main()
{
	gl_Position = vec4(vaPosition, 0.0, 1.0);
	vso_TexCoord = vaTexCoord;
}
