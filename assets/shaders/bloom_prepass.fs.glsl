#version 450 core

out vec4 FragColor;

layout(binding = 0) uniform sampler2D image;

uniform float g_fTreshold;

void main()
{
    vec2 texcoord = gl_FragCoord.xy / textureSize(image, 0);
    vec4 c = texture(image, texcoord);
    
    float brightness = dot(c.rgb, vec3(0.2126, 0.7152, 0.0722));

    FragColor = (brightness > g_fTreshold) ? vec4(c.rgb, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);

}