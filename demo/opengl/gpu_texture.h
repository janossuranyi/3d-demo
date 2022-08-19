#pragma once

#include <GL/glew.h>
#include <cinttypes>
#include <utility>
#include <vector>
#include <functional>
#include "gpu_types.h"

class GpuTexture
{
public:
	GpuTexture() : mTexture(INVALID_TEXTURE) {}
	virtual bool create(int w, int h, int level, eTextureFormat internalFormat, ePixelFormat format, eDataType type, const void** data) = 0;
	virtual void bind() const = 0;
//	virtual void bindImage(int unit, int level, bool layered, int layer, eImageAccess access, eImageFormat format) = 0;
	virtual eTextureTarget getTarget() const = 0;
	GpuTexture& withMinFilter(eTexMinFilter p);
	GpuTexture& withMagFilter(eTexMagFilter p);
	GpuTexture& withWrapS(eTexWrap p);
	GpuTexture& withWrapT(eTexWrap p);
	GpuTexture& withWrapR(eTexWrap p);
	GpuTexture& withDefaultLinearRepeat();
	GpuTexture& withDefaultLinearClampEdge();
	GpuTexture& withDefaultMipmapRepeat();
	GpuTexture& withDefaultMipmapClampEdge();

	void updateParameters();
	void selectUnit(int unit) const;
	void generateMipMaps() const;

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
	~GpuTexture2D();
	bool create(int w, int h, int level, eTextureFormat internalFormat, ePixelFormat format, eDataType type, const void** data) override;
	eTextureTarget getTarget() const override { return eTextureTarget::TEX_2D; }
	void bind() const override;
	void bindImage(int unit, int level, eImageAccess access, eImageFormat format);
protected:
	inline GLenum getApiTarget() const override { return GL_TEXTURE_2D; };

};

class GpuTextureCubeMap : public GpuTexture
{
public:
	GpuTextureCubeMap() : GpuTexture() {}
	~GpuTextureCubeMap();
	bool create(int w, int h, int level, eTextureFormat internalFormat, ePixelFormat format, eDataType type, const void** data) override;
	bool create(unsigned int side, int w, int h, int level, eTextureFormat internalFormat, ePixelFormat format, eDataType type, const void* data);
	bool create(int level, eTextureFormat internalFormat, ePixelFormat format, eDataType type,
		std::function<void(int,int&,int&,int&,void**)> getImageCB,
		std::function<void(int,void*)> freeImageCB);
	eTextureTarget getTarget() const override { return eTextureTarget::TEX_CUBE_MAP; }
	void bind() const override;
	void bindImage(int unit, int level, bool layered, int layer, eImageAccess access, eImageFormat format);
protected:
	inline GLenum getApiTarget() const override { return GL_TEXTURE_CUBE_MAP; };

};