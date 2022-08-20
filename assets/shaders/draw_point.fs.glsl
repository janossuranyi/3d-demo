#version 450 core

in vec4 vso_Color;

layout(location = 0) out vec4 fragColor;

void main() {
	fragColor = vso_Color;
}
