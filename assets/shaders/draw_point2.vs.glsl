//#extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec4 va_position;
layout(location = 1) in vec2 va_texcoord;
layout(location = 2) in vec4 va_normal;
layout(location = 3) in vec4 va_tangent;
layout(location = 4) in vec4 va_color;

uniform mat4 m_WVP;

out INTERFACE {
	vec4 fragColor;
} Out;


void main() {
	Out.fragColor = va_color;
	gl_Position = m_WVP * va_position;
	gl_PointSize = clamp(20 - (gl_Position.z / 50), 1, 20);
}
