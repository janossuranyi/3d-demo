#pragma once

#include <GL/glew.h>
#include <cinttypes>
#include <utility>
#include <vector>
#include <functional>
#include <memory>
#include "gpu_types.h"
//#include "pipeline.h"

#define STD_TEXTURE_METHODS(T) \
	T (T&) = delete; \
	T (T&&) = delete; \
	T& operator=(T&) = delete; \
	T& operator=(T&&) = delete;

class GpuTexture
{
	friend class GpuFrameBuffer;
	friend class Pipeline;
public:
	GpuTexture() :
		mTexture(INVALID_TEXTURE),
		m_width(),
		m_height(),
		m_depth() {}
	STD_TEXTURE_METHODS(GpuTexture)

	virtual void bind() const = 0;
	virtual void bind(int unit) const = 0;
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
	friend class Pipeline;
public:
	using Ptr = std::shared_ptr<GpuTexture2D>;

	GpuTexture2D() : GpuTexture() {}
	~GpuTexture2D();
	STD_TEXTURE_METHODS(GpuTexture2D)

	bool create(int w, int h, int level, eTextureFormat internalFormat, ePixelFormat format, eDataType type, const void* data);
	bool createFromImage(const std::string& fromFile, bool srgb = false, bool autoMipmap = true, bool compress = true);
	bool createRGB(int w, int h, int level);
	bool createRGB8(int w, int h, int level);
	bool createRGB8S(int w, int h, int level);
	bool createRG8U(int w, int h, int level);
	bool createRG8S(int w, int h, int level);
	bool createRG16U(int w, int h, int level);
	bool createRG16S(int w, int h, int level);
	bool createRG16F(int w, int h, int level);
	bool createRGB32F(int w, int h, int level);
	bool createRGB16F(int w, int h, int level);
	bool createRGB10A2(int w, int h, int level);
	bool createR11G11B10(int w, int h, int level);
	bool createDepthStencil(int w, int h);
	eTextureTarget getTarget() const override { return eTextureTarget::TEX_2D; }
	void bind() const override;
	void bind(int unit) const override;
	void bindImage(int unit, int level, eImageAccess access, eImageFormat format);

	static GpuTexture2D::Ptr createShared();

protected:
	inline GLenum getApiTarget() const override { return GL_TEXTURE_2D; };

};

class GpuTextureCubeMap : public GpuTexture
{
	friend class GpuFrameBuffer;
	friend class Pipeline;
public:
	using Ptr = std::shared_ptr<GpuTextureCubeMap>;

	GpuTextureCubeMap() : GpuTexture() {}
	~GpuTextureCubeMap();
	STD_TEXTURE_METHODS(GpuTextureCubeMap)

	bool createFromImage(const std::vector<std::string>& fromFile, bool srgb = false, bool autoMipmap = false, bool compress = true);

	eTextureTarget getTarget() const override { return eTextureTarget::TEX_CUBE_MAP; }
	void bind() const override;
	void bind(int unit) const override;
	void bindImage(int unit, int level, bool layered, int layer, eImageAccess access, eImageFormat format);
protected:
	inline GLenum getApiTarget() const override { return GL_TEXTURE_CUBE_MAP; };

};