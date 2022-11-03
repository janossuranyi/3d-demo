#include "version.inc.glsl"

layout(location = 0) in vec4 va_position;
layout(location = 1) in vec2 va_texcoord;
layout(location = 2) in vec4 va_normal;
layout(location = 3) in vec4 va_tangent;
layout(location = 4) in vec4 va_color;

out INTERFACE {
	vec2 TexCoord;
} Out;

void main() {
	gl_Position = va_position;
	Out.TexCoord = va_texcoord;
}
