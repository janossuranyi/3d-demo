#version 430 core

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_TexCoord;

out INTERFACE {
	vec2 uv; ///< UV coordinates.
} Out;

void main() {
	gl_Position = vec4(in_Position, 0.0, 1.0);
	Out.uv = in_TexCoord;
}
