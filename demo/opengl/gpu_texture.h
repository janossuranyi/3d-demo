#pragma once

#include <GL/glew.h>
#include <cinttypes>
#include "gl_common.h"

enum class eTextureTarget { TEX_1D, TEX_2D, TEX_3D, TEX_CUBE_MAP, ENUM_SIZE };
enum class eTextureFormat { RGB, RGBA, RGBA16F, RGBA32F, DEPTH24_STENCIL_8, ENUM_SITZE };

class GpuTexture
{
public:
	GpuTexture();
	GpuTexture(GpuTexture&) = delete;
	~GpuTexture();
	GpuTexture& operator=(GpuTexture&) = delete;
	bool Create2D(uint32_t width, uint32_t height, DataType format);
private:
	GLuint mTexture;
	GLuint mTarget;
	eTextureTarget mType;
};