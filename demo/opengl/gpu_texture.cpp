#include <utility>
#include <cassert>
#include <SOIL2.h>
#include "logger.h"
#include "gpu_utils.h"
#include "gpu_texture.h"

GpuTexture& GpuTexture::withMinFilter(eTexMinFilter p)
{
    GLint value;

    switch (p)
    {
        case eTexMinFilter::LINEAR:                 value = GL_LINEAR; break;
        case eTexMinFilter::NEAREST:                value = GL_NEAREST; break;
        case eTexMinFilter::LINEAR_MIPMAP_LINEAR:   value = GL_LINEAR_MIPMAP_LINEAR; break;
        case eTexMinFilter::LINEAR_MIPMAP_NEAREST:  value = GL_LINEAR_MIPMAP_NEAREST; break;
        case eTexMinFilter::NEAREST_MIPMAP_NEAREST: value = GL_NEAREST_MIPMAP_NEAREST; break;
        case eTexMinFilter::NEAREST_MIPMAP_LINEAR:  value = GL_NEAREST_MIPMAP_LINEAR; break;
        default: value = GL_LINEAR;
    }

    mIntParams.push_back(std::make_pair(GL_TEXTURE_MIN_FILTER, value));

    return *this;
}

GpuTexture& GpuTexture::withMagFilter(eTexMagFilter p)
{
    GLint value;

    switch (p)
    {
    case eTexMagFilter::LINEAR:                 value = GL_LINEAR; break;
    case eTexMagFilter::NEAREST:                value = GL_NEAREST; break;
    default: value = GL_LINEAR;
    }

    mIntParams.push_back(std::make_pair(GL_TEXTURE_MAG_FILTER, value));

    return *this;
}

GpuTexture& GpuTexture::withWrapS(eTexWrap p)
{
    GLint value = GL_castTexWrap(p);
    mIntParams.push_back(std::make_pair(GL_TEXTURE_WRAP_S, value));
    
    return *this;
}

GpuTexture& GpuTexture::withWrapT(eTexWrap p)
{
    GLint value = GL_castTexWrap(p);
    mIntParams.push_back(std::make_pair(GL_TEXTURE_WRAP_T, value));

    return *this;
}

GpuTexture& GpuTexture::withWrapR(eTexWrap p)
{
    GLint value = GL_castTexWrap(p);
    mIntParams.push_back(std::make_pair(GL_TEXTURE_WRAP_R, value));

    return *this;
}

GpuTexture& GpuTexture::withDefaultLinearRepeat()
{
    mIntParams.push_back(std::make_pair(GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    mIntParams.push_back(std::make_pair(GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    mIntParams.push_back(std::make_pair(GL_TEXTURE_WRAP_S, GL_REPEAT));
    mIntParams.push_back(std::make_pair(GL_TEXTURE_WRAP_T, GL_REPEAT));
    if (getTarget() == eTextureTarget::TEX_CUBE_MAP) {
        mIntParams.push_back(std::make_pair(GL_TEXTURE_WRAP_R, GL_REPEAT));
    }

    return *this;
}

GpuTexture& GpuTexture::withDefaultLinearClampEdge()
{
    mIntParams.push_back(std::make_pair(GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    mIntParams.push_back(std::make_pair(GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    mIntParams.push_back(std::make_pair(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    mIntParams.push_back(std::make_pair(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    if (getTarget() == eTextureTarget::TEX_CUBE_MAP) {
        mIntParams.push_back(std::make_pair(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    }

    return *this;
}

GpuTexture& GpuTexture::withDefaultMipmapRepeat()
{
    mIntParams.push_back(std::make_pair(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    mIntParams.push_back(std::make_pair(GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    mIntParams.push_back(std::make_pair(GL_TEXTURE_WRAP_S, GL_REPEAT));
    mIntParams.push_back(std::make_pair(GL_TEXTURE_WRAP_T, GL_REPEAT));
    if (getTarget() == eTextureTarget::TEX_CUBE_MAP) {
        mIntParams.push_back(std::make_pair(GL_TEXTURE_WRAP_R, GL_REPEAT));
    }

    return *this;
}

GpuTexture& GpuTexture::withDefaultMipmapClampEdge()
{
    mIntParams.push_back(std::make_pair(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    mIntParams.push_back(std::make_pair(GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    mIntParams.push_back(std::make_pair(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    mIntParams.push_back(std::make_pair(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    if (getTarget() == eTextureTarget::TEX_CUBE_MAP) {
        mIntParams.push_back(std::make_pair(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    }

    return *this;
}

void GpuTexture::updateParameters()
{
    for (auto& p : mIntParams)
    {
        GL_CHECK(glTexParameteri(getApiTarget(), p.first, p.second));
    }

    for (auto& p : mFloatParams)
    {
        GL_CHECK(glTexParameterf(getApiTarget(), p.first, p.second));
    }

    mIntParams.clear();
    mFloatParams.clear();
}

void GpuTexture::selectUnit(int unit) const
{
    GL_CHECK(glActiveTexture(GL_TEXTURE0 + unit));
}

void GpuTexture::generateMipMaps() const
{
    GL_CHECK(glGenerateMipmap(getApiTarget()));
}

void GpuTexture::getDimensions(unsigned int& w, unsigned int& h, unsigned int& d)
{
    w = m_width;
    h = m_height;
    d = m_depth;
}

GpuTexture2D::~GpuTexture2D()
{
    if (mTexture != INVALID_TEXTURE)
        GL_CHECK(glDeleteTextures(1, &mTexture));
}

bool GpuTexture2D::create(int w, int h, int level, eTextureFormat internalFormat, ePixelFormat format, eDataType type, const void* data)
{
    if (mTexture == INVALID_TEXTURE) GL_CHECK(glGenTextures(1, &mTexture));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, mTexture));
    
    const GLenum dataType = GL_castDataType(type);
    const GLenum pixFormat = GL_castPixelFormat(format);
    const GLint texFormat = GL_castTextureFormat(internalFormat);

    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, level, texFormat, w, h, 0, pixFormat, dataType, (data ? data : nullptr)));

    return true;
}

bool GpuTexture2D::createFromImage(const std::string& fromFile, bool srgb, bool autoMipmap, bool compress)
{
    GLuint texID = mTexture != INVALID_TEXTURE ? mTexture : SOIL_CREATE_NEW_ID;
    unsigned int flags = 0;
    if (autoMipmap)
    {
        flags |= SOIL_FLAG_MIPMAPS;
    }
    if (compress)
    {
        flags |= SOIL_FLAG_COMPRESS_TO_DXT;
    }
    if (srgb)
    {
        flags |= SOIL_FLAG_SRGB_COLOR_SPACE;
    }

    texID = SOIL_load_OGL_texture(fromFile.c_str(), 0, texID, flags);
    if (texID) mTexture = texID;

    return texID != 0;

}

bool GpuTexture2D::createFromMemory(const void* data, uint32_t bufLen, bool srgb, bool autoMipmap, bool compress)
{
    GLuint texID = mTexture != INVALID_TEXTURE ? mTexture : SOIL_CREATE_NEW_ID;
    unsigned int flags = 0;
    if (autoMipmap)
    {
        flags |= SOIL_FLAG_MIPMAPS;
    }
    if (compress)
    {
        flags |= SOIL_FLAG_COMPRESS_TO_DXT;
    }
    if (srgb)
    {
        flags |= SOIL_FLAG_SRGB_COLOR_SPACE;
    }

    texID = SOIL_load_OGL_texture_from_memory((unsigned char*)data, bufLen, 0, texID, flags);
    if (texID) mTexture = texID;
    
    return texID != 0;
}

bool GpuTexture2D::createRGB(int w, int h, int level)
{
    return createRGB8(w, h, level);
}

bool GpuTexture2D::createRG8U(int w, int h, int level)
{
    return create(w, h, level, eTextureFormat::RG, ePixelFormat::RGB, eDataType::UNSIGNED_BYTE, nullptr);
}

bool GpuTexture2D::createRG8S(int w, int h, int level)
{
    return create(w, h, level, eTextureFormat::RG, ePixelFormat::RGB, eDataType::BYTE, nullptr);
}

bool GpuTexture2D::createRG16U(int w, int h, int level)
{
    return create(w, h, level, eTextureFormat::RG16, ePixelFormat::RGB, eDataType::UNSIGNED_SHORT, nullptr);
}

bool GpuTexture2D::createRG16S(int w, int h, int level)
{
    return create(w, h, level, eTextureFormat::RG16, ePixelFormat::RGB, eDataType::SHORT, nullptr);
}

bool GpuTexture2D::createRG16F(int w, int h, int level)
{
    return create(w, h, level, eTextureFormat::RG16F, ePixelFormat::RGB, eDataType::FLOAT, nullptr);
}

bool GpuTexture2D::createRGB8(int w, int h, int level)
{
    return create(w, h, level, eTextureFormat::RGBA, ePixelFormat::RGB, eDataType::UNSIGNED_BYTE, nullptr);
}

bool GpuTexture2D::createRGB8S(int w, int h, int level)
{
    return create(w, h, level, eTextureFormat::RGBA, ePixelFormat::RGB, eDataType::BYTE, nullptr);
}

bool GpuTexture2D::createRGB32F(int w, int h, int level)
{
    return create(w, h, level, eTextureFormat::RGBA32F, ePixelFormat::RGB, eDataType::FLOAT, nullptr);
}

bool GpuTexture2D::createRGB16F(int w, int h, int level)
{
    return create(w, h, level, eTextureFormat::RGBA16F, ePixelFormat::RGB, eDataType::FLOAT, nullptr);
}

bool GpuTexture2D::createRGB10A2(int w, int h, int level)
{
    return create(w, h, level, eTextureFormat::RGB10A2, ePixelFormat::RGB, eDataType::FLOAT, nullptr);
}

bool GpuTexture2D::createR11G11B10(int w, int h, int level)
{
    return create(w, h, level, eTextureFormat::R11F_G11F_B10F, ePixelFormat::RGB, eDataType::FLOAT, nullptr);
}

bool GpuTexture2D::createDepthStencil(int w, int h)
{
    if (mTexture == INVALID_TEXTURE)
    {
        GL_CHECK(glGenTextures(1, &mTexture));
    }

    GL_CHECK(glBindTexture(GL_TEXTURE_2D, mTexture));

    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, w, h, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    return true;
}

GpuTexture2D::Ptr GpuTexture2D::createShared()
{
    return std::make_shared<GpuTexture2D>();
}

void GpuTexture2D::bind() const
{
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, mTexture));
}

void GpuTexture2D::bind(int unit) const
{
    if (GLEW_VERSION_4_5)
    {
        GL_CHECK(glBindTextureUnit(unit, mTexture));
    }
    else
    {
        GL_CHECK(glActiveTexture(GL_TEXTURE0 + unit));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, mTexture));
    }
}

void GpuTexture2D::bindImage(int unit, int level, eImageAccess access, eImageFormat format)
{
    GLenum access_ = GL_castImageAccess(access);
    GLenum format_ = GL_castImageFormat(format);

    GL_CHECK(glBindImageTexture(GLuint(unit), mTexture, GLint(level), GL_FALSE, 0, access_, format_));
}

GpuTextureCubeMap::~GpuTextureCubeMap()
{
    if (mTexture != INVALID_TEXTURE)
        GL_CHECK(glDeleteTextures(1, &mTexture));
}


bool GpuTextureCubeMap::createFromImage(const std::vector<std::string>& fromFile, bool srgb, bool autoMipmap, bool compress)
{
    GLuint texID = mTexture != INVALID_TEXTURE ? mTexture : SOIL_CREATE_NEW_ID;
    unsigned int flags = 0;
    if (autoMipmap)
    {
        flags |= SOIL_FLAG_MIPMAPS;
    }
    if (compress)
    {
        flags |= SOIL_FLAG_COMPRESS_TO_DXT;
    }
    if (srgb)
    {
        flags |= SOIL_FLAG_SRGB_COLOR_SPACE;
    }

    texID = SOIL_load_OGL_cubemap(
        fromFile[0].c_str(),
        fromFile[1].c_str(),
        fromFile[2].c_str(),
        fromFile[3].c_str(),
        fromFile[4].c_str(),
        fromFile[5].c_str(), 0, texID, flags);

    if (texID) mTexture = texID;

    return texID != 0;
}

void GpuTextureCubeMap::bind() const
{
    GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture));
}

void GpuTextureCubeMap::bind(int unit) const
{
    if (GLEW_VERSION_4_5)
    {
        GL_CHECK(glBindTextureUnit(unit, mTexture));
    }
    else
    {
        GL_CHECK(glActiveTexture(GL_TEXTURE0 + unit));
        GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture));
    }
}

void GpuTextureCubeMap::bindImage(int unit, int level, bool layered, int layer, eImageAccess access, eImageFormat format)
{
    GLenum access_ = GL_castImageAccess(access);
    GLenum format_ = GL_castImageFormat(format);

    GL_CHECK(glBindImageTexture(GLuint(unit), mTexture, GLint(level), GLboolean(layered), GLint(layer), access_, format_));
}
