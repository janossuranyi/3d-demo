#version 330 core

layout(location = 0) in vec4 va_position;
layout(location = 1) in vec4 va_qtangent;
layout(location = 2) in vec2 va_st;
layout(location = 3) in vec4 va_color;

out vec4 vo_position;
out vec4 vo_qtangent;
out vec2 vo_st;
out vec4 vo_color;

void main() {
	gl_Position = va_position;

	vo_position = va_position;
	vo_qtangent = va_qtangent;
	vo_st = va_st;
	vo_color = va_color;
} 
