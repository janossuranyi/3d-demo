#include "version.inc.glsl"

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_Texcoord;

out INTERFACE {
	vec2 TexCoord;
} Out;

uniform mat4 m_W;

void main() {
	gl_Position = m_W * in_Position;
	Out.TexCoord = in_Texcoord;
}
