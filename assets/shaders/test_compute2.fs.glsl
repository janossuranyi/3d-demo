
in INTERFACE {
	vec2 TexCoord;
} In;

out vec4 FS_OUT;

uniform sampler2D samp0;

void main()
{
	FS_OUT = texture(samp0, In.TexCoord);
}