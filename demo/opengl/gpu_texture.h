#pragma once

#include <GL/glew.h>
#include <cinttypes>
#include <utility>
#include <vector>
#include "gpu_types.h"

class GpuTexture
{
public:
	GpuTexture() : mTexture(INVALID_TEXTURE) {}
	virtual bool create(int w, int h, int level, eTextureFormat internalFormat, ePixelFormat format, eDataType type, const void** data) = 0;
	virtual void bind() const = 0;
	virtual eTextureTarget getTarget() const = 0;
	GpuTexture& withMinFilter(eTexMinFilter p);
	GpuTexture& withMagFilter(eTexMagFilter p);
	GpuTexture& withWrapS(eTexWrap p);
	GpuTexture& withWrapT(eTexWrap p);
	GpuTexture& withWrapR(eTexWrap p);

	void updateParameters();

protected:
	virtual GLenum getApiTarget() const = 0;

	GLuint mTexture;

	using IntegerParamsVec = std::vector<std::pair<GLenum, GLint>>;
	using FloatParamsVec = std::vector<std::pair<GLenum, GLfloat>>;

	IntegerParamsVec mIntParams;
	FloatParamsVec mFloatParams;
};

class GpuTexture2D : public GpuTexture
{
public:
	GpuTexture2D() : GpuTexture() {}
	~GpuTexture2D() {}
	bool create(int w, int h, int level, eTextureFormat internalFormat, ePixelFormat format, eDataType type, const void** data) override;
	eTextureTarget getTarget() const override { return eTextureTarget::TEX_2D; }
	void bind() const override;
protected:
	inline GLenum getApiTarget() const override { return GL_TEXTURE_2D; };

};

class GpuTextureCubeMap : public GpuTexture
{
public:
	GpuTextureCubeMap() : GpuTexture() {}
	~GpuTextureCubeMap() {}
	eTextureTarget getTarget() const override { return eTextureTarget::TEX_CUBE_MAP; }
	void bind() const override {}
protected:
	inline GLenum getApiTarget() const override { return GL_TEXTURE_CUBE_MAP; };

};