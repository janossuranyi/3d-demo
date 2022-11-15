#include "version.inc.glsl"

//#extension GL_ARB_explicit_uniform_location : enable

layout(location = 0) in vec4 in_Position;
layout(location = 4) in vec4 in_Color;
layout(location = 6) in float in_Instance;

uniform mat4 g_mWorldTransform;
uniform mat4 g_mViewProjectionTransform;
uniform mat4 g_mWorldTransform2;

out INTERFACE {
	vec4 fragColor;
} Out;


void main() {

	Out.fragColor = in_Color;

	//gl_InstanceID
	
	mat4 W = (in_Instance < 0.5) ? g_mWorldTransform2 : g_mWorldTransform;
	//mat4 W = (gl_InstanceID % 2)==0 ? g_mWorldTransform2 : g_mWorldTransform;

	gl_Position = g_mViewProjectionTransform * W * in_Position;
	gl_PointSize = clamp(20 - (gl_Position.z / 50), 1, 20);
}
