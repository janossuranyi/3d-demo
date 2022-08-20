#pragma once

#include <GL/glew.h>
#include <cinttypes>
#include <utility>
#include <vector>
#include <functional>
#include "gpu_types.h"

#define STD_METHODS(T) \
	T (T&) = delete; \
	T (T&&) = delete; \
	T& operator=(T&) = delete; \
	T& operator=(T&&) = delete;

class GpuTexture
{
	friend class GpuFrameBuffer;
public:
	GpuTexture() :
		mTexture(INVALID_TEXTURE),
		m_width(),
		m_height() {}
	STD_METHODS(GpuTexture)

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

	unsigned int textureID() const { return mTexture; }
	void getDimensions(unsigned int& w, unsigned int& h, unsigned int& d);
protected:
	virtual GLenum getApiTarget() const = 0;

	GLuint mTexture;

	using IntegerParamsVec = std::vector<std::pair<GLenum, GLint>>;
	using FloatParamsVec = std::vector<std::pair<GLenum, GLfloat>>;

	IntegerParamsVec mIntParams;
	FloatParamsVec mFloatParams;

	unsigned int m_width, m_height, m_depth;
};

class GpuTexture2D : public GpuTexture
{
	friend class GpuFrameBuffer;
public:
	GpuTexture2D() : GpuTexture() {}
	~GpuTexture2D();
	STD_METHODS(GpuTexture2D)

	bool create(int w, int h, int level, eTextureFormat internalFormat, ePixelFormat format, eDataType type, const void* data);
	bool createFromImage(const std::string& fromFile, bool srgb = false, bool autoMipmap = true, bool compress = true);
	bool createRGB8(int w, int h, int level);
	bool createRGB32F(int w, int h, int level);
	bool createRGB16F(int w, int h, int level);
	bool createDepthStencil(int w, int h);

	eTextureTarget getTarget() const override { return eTextureTarget::TEX_2D; }
	void bind() const override;
	void bindImage(int unit, int level, eImageAccess access, eImageFormat format);

protected:
	inline GLenum getApiTarget() const override { return GL_TEXTURE_2D; };

};

class GpuTextureCubeMap : public GpuTexture
{
	friend class GpuFrameBuffer;
public:
	GpuTextureCubeMap() : GpuTexture() {}
	~GpuTextureCubeMap();
	STD_METHODS(GpuTextureCubeMap)

	bool create(int w, int h, int level, eTextureFormat internalFormat, ePixelFormat format, eDataType type, const void** data);
	bool create(unsigned int side, int w, int h, int level, eTextureFormat internalFormat, ePixelFormat format, eDataType type, const void* data);
	bool create(int level, eTextureFormat internalFormat, ePixelFormat format, eDataType type,
		std::function<void(int,int&,int&,int&,void**)> getImageCB,
		std::function<void(int,void*)> freeImageCB);

	bool createFromImage(const std::vector<std::string>& fromFile, bool srgb = false, bool autoMipmap = false, bool compress = true);

	eTextureTarget getTarget() const override { return eTextureTarget::TEX_CUBE_MAP; }
	void bind() const override;
	void bindImage(int unit, int level, bool layered, int layer, eImageAccess access, eImageFormat format);
protected:
	inline GLenum getApiTarget() const override { return GL_TEXTURE_CUBE_MAP; };

};