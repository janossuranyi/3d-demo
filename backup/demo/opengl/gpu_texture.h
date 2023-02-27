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
	T& operator=(T&) = delete; \

class GpuTexture
{
	friend class GpuFrameBuffer;
	friend class GPU;
	friend class Pipeline;
public:
	~GpuTexture() noexcept;
	GpuTexture() : mTexture(INVALID_TEXTURE){}

	STD_TEXTURE_METHODS(GpuTexture)

	GpuTexture(GpuTexture&& moved) = delete;
	GpuTexture& operator=(GpuTexture&& moved) = delete;

	//	virtual void bindImage(int unit, int level, bool layered, int layer, eImageAccess access, eImageFormat format) = 0;
	virtual TextureShape getTarget() const = 0;
	GpuTexture& withMinFilter(FilterMin p);
	GpuTexture& withMagFilter(FilterMag p);
	GpuTexture& withWrapS(Wrap p);
	GpuTexture& withWrapT(Wrap p);
	GpuTexture& withWrapR(Wrap p);
	GpuTexture& withDefaultLinearRepeat();
	GpuTexture& withDefaultLinearClampEdge();
	GpuTexture& withDefaultMipmapRepeat();
	GpuTexture& withDefaultMipmapClampEdge();
	void updateParameters();
	void selectUnit(int unit) const;
	void generateMipMaps() const;

	unsigned int textureID() const { return mTexture; }
protected:
	virtual GLenum getApiTarget() const = 0;

	GLuint mTexture;

	using IntegerParamsVec = std::vector<std::pair<GLenum, GLint>>;
	using FloatParamsVec = std::vector<std::pair<GLenum, GLfloat>>;

	IntegerParamsVec _intParams;
	FloatParamsVec _floatParams;

	static void getTextureFormats(int channels, bool srgb, bool compress, InternalFormat& internalFormat, InputFormat& format);

//	unsigned int m_width, m_height, m_depth;
};

class GpuTexture2D : public GpuTexture
{
	friend class GpuFrameBuffer;
	friend class Pipeline;
public:

	GpuTexture2D() : GpuTexture() {}
	~GpuTexture2D();

	GpuTexture2D(GpuTexture2D&& moved) noexcept;
	GpuTexture2D& operator=(GpuTexture2D&& moved) noexcept;

	STD_TEXTURE_METHODS(GpuTexture2D)

	bool create(int w, int h, int level, InternalFormat internalFormat, InputFormat format, ComponentType type, const void* data);
	bool createFromImage(const std::string& fromFile, bool srgb = false, bool autoMipmap = true, bool compress = true);
	bool createFromMemory(const void* data, size_t bufLen, bool srgb = false, bool autoMipmap = true, bool compress = true);
	bool createRGB(int w, int h, int level);
	bool createRGB565(int w, int h, int level);
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
	TextureShape getTarget() const override { return TextureShape::D2; }

	static std::shared_ptr<GpuTexture2D> createShared();

protected:
	inline GLenum getApiTarget() const override { return GL_TEXTURE_2D; };
};

class GpuTextureCubeMap : public GpuTexture
{
	friend class GpuFrameBuffer;
	friend class Pipeline;
public:

	GpuTextureCubeMap() : GpuTexture() {}
	~GpuTextureCubeMap();
	STD_TEXTURE_METHODS(GpuTextureCubeMap)

	bool createFromImage(const std::vector<std::string>& fromFile, bool srgb = false, bool autoMipmap = false, bool compress = true);

	TextureShape getTarget() const override { return TextureShape::CUBE_MAP; }
protected:
	inline GLenum getApiTarget() const override { return GL_TEXTURE_CUBE_MAP; };
	bool cubemapHelper(const std::string& fileName, uint index, bool srgb, bool compress);

};