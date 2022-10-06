#pragma once

#include <SDL.h>
#include "window.h"
#include "gpu_buffer.h"
#include "gpu_texture.h"
#include "gpu_utils.h"

class GPU
{
public:
	static bool setup(const Window& awindow, const std::string& name);
	static void close();
	static const Window& window();
	static bool valid();
	static void flipSwapChain();
	static void bind(const GpuBuffer& abuf);
	static void bind(const GpuTexture& atex);
	static void bind(const GpuTexture& atex, unsigned int aunit);
	static void activeTexture(unsigned int aunit);
	static void bindImageUnit(const GpuTexture& atex, int unit, int level, Access access, ImageFormat format);
	static void createTexture2D(GpuTexture2D& atex, int w, int h, int level, InternalFormat internalFormat, InputFormat format, ComponentType type, const void* data);
	static void createTextureCubeface(GpuTexture& atex, int w, int h, int level, int face, InternalFormat internalFormat, InputFormat format, ComponentType type, const void* data);
	static void generateMipmap(const GpuTexture& atex);
	static void generateMipmapCubeface(const GpuTexture& atex, int face);
	static void resetTexture1D();
	static void resetTexture2D();
	static void resetTextureCube();
	static void updateTextureParameters(GpuTexture& atex);

	static void deleteTexture(GpuTexture& atex);
private:
	static SDL_Window* _windowHandle;
	static SDL_GLContext _glcontext;
	static Window _window;
	static bool _valid;
	static int _glVersion;
};