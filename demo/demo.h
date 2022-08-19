#pragma once

#include <SDL.h>
#include <GL/glew.h>

struct VideoConfig
{
	int width;
	int height;
	int redBits;
	int greenBits;
	int blueBits;
	int depthBits;
	int stencilBits;
	int multisample;
	int glVersion;
	bool explicitUnifromLocationEXT{ false };
	SDL_Window* hWindow{};
	SDL_GLContext hGl{};

};

extern VideoConfig videoConf;
