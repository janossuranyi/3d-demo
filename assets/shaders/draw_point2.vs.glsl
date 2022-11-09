#include "version.inc.glsl"

//#extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec4 in_Position;
layout(location = 4) in vec4 in_Color;
layout(location = 5) in float in_Instance;

uniform mat4 m_W;
uniform mat4 m_VP;
uniform mat4 m_W2;

out INTERFACE {
	vec4 fragColor;
} Out;


void main() {

	Out.fragColor = in_Color;

	//gl_InstanceID
	
	mat4 W = (in_Instance < 0.5) ? m_W2 : m_W;
	//mat4 W = (gl_InstanceID % 2)==0 ? m_W2 : m_W;

	gl_Position = m_VP * W * in_Position;
	gl_PointSize = clamp(20 - (gl_Position.z / 50), 1, 20);
}
