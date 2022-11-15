#include "version.inc.glsl"

in INTERFACE {
	vec2 uv;
} In;

out vec4 FS_OUT;

uniform sampler2D g_tInput;

void main()
{
	FS_OUT = texture(g_tInput, In.uv);
}