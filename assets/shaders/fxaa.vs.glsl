#version 430 core

layout(location = 0) in vec2 vaPosition;
layout(location = 1) in vec2 vaTexCoord;

out INTERFACE {
	vec2 uv; ///< UV coordinates.
} Out;

void main() {
	gl_Position = vec4(vaPosition, 0.0, 1.0);
	Out.uv = vaTexCoord;
}
