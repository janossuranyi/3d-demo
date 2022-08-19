#version 330 core

layout(location = 0) in vec4 va_position;
layout(location = 1) in vec4 va_qtangent;
layout(location = 2) in vec2 va_st;
layout(location = 3) in vec4 va_color;

void main()
{
  gl_Position = va_position;
}
