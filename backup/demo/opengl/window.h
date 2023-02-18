#pragma once

struct Window
{
	int width;
	int height;
	int depthBits;
	int colorBits;
	int stencilBits;
	int multisample;
	int redBits;
	int greenBits;
	int blueBits;
	int alphaBits;
	bool fullscreen;
};