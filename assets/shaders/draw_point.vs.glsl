#version 330 core
//#extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec3 vaPosition;
layout(location = 1) in vec4 vaColor;

uniform mat4 m_WVP;
out vec4 vso_Color;

void main() {
	vso_Color = vaColor;
	gl_Position = m_WVP * vec4(vaPosition, 1.0);
	gl_PointSize = clamp(20 - (gl_Position.z / 50), 1, 20);
}
