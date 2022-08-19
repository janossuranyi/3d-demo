#include <utility>
#include <cassert>
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

GpuTexture2D::~GpuTexture2D()
{
    if (mTexture != INVALID_TEXTURE)
        GL_CHECK(glDeleteTextures(1, &mTexture));
}

bool GpuTexture2D::create(int w, int h, int level, eTextureFormat internalFormat, ePixelFormat format, eDataType type, const void** data)
{
    if (mTexture == INVALID_TEXTURE) GL_CHECK(glGenTextures(1, &mTexture));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, mTexture));
    
    const GLenum dataType = GL_castDataType(type);
    const GLenum pixFormat = GL_castPixelFormat(format);
    const GLint texFormat = GL_castTextureFormat(internalFormat);

    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, level, texFormat, w, h, 0, pixFormat, dataType, (data ? *data : nullptr)));

    return true;
}

void GpuTexture2D::bind() const
{
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, mTexture));
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

bool GpuTextureCubeMap::create(int w, int h, int level, eTextureFormat internalFormat, ePixelFormat format, eDataType type, const void** data)
{
    if (mTexture == INVALID_TEXTURE) GL_CHECK(glGenTextures(1, &mTexture));
    GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture));

    const GLenum dataType = GL_castDataType(type);
    const GLenum pixFormat = GL_castPixelFormat(format);
    const GLint texFormat = GL_castTextureFormat(internalFormat);

    if (!data)
    {
        for (int k = 0; k < 6; ++k)
        {
            GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + k, level, texFormat, w, h, 0, pixFormat, dataType, nullptr));
        }
    }
    else
    {
        for (int k = 0; k < 6; ++k)
        {
            assert(data[k] != nullptr);
            GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + k, level, texFormat, w, h, 0, pixFormat, dataType, data[k]));
        }
    }

    return true;
}

bool GpuTextureCubeMap::create(unsigned int side, int w, int h, int level, eTextureFormat internalFormat, ePixelFormat format, eDataType type, const void* data)
{
    assert(data != nullptr);

    if (mTexture == INVALID_TEXTURE)
    {
        GL_CHECK(glGenTextures(1, &mTexture));
        GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture));
    }

    const GLenum dataType = GL_castDataType(type);
    const GLenum pixFormat = GL_castPixelFormat(format);
    const GLint texFormat = GL_castTextureFormat(internalFormat);

    GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, level, texFormat, w, h, 0, pixFormat, dataType, data));

    return false;
}

bool GpuTextureCubeMap::create(int level, eTextureFormat internalFormat, ePixelFormat format, eDataType type,
    std::function<void(int, int&, int&, int&, void**)> getImageCB,
    std::function<void(int, void*)> freeImageCB)
{
    if (mTexture == INVALID_TEXTURE) GL_CHECK(glGenTextures(1, &mTexture));
    GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture));
    const GLenum dataType = GL_castDataType(type);
    const GLint texFormat = GL_castTextureFormat(internalFormat);
    GLenum pixFormat = GL_castPixelFormat(format);

    int w, h, n;
    for (int k = 0; k < 6; ++k)
    {
        void* data = 0;
        w = h = n = 0;

        getImageCB(k, w, h, n, &data);

        if (data && w > 0 && h > 0 && n >= 3)
        {
            pixFormat = n == 3 ? GL_RGB : GL_RGBA;
            GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + k, level, texFormat, w, h, 0, pixFormat, dataType, data));
            freeImageCB(k, data);
        }
    }
    return true;
}

void GpuTextureCubeMap::bind() const
{
    GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture));
}

void GpuTextureCubeMap::bindImage(int unit, int level, bool layered, int layer, eImageAccess access, eImageFormat format)
{
    GLenum access_ = GL_castImageAccess(access);
    GLenum format_ = GL_castImageFormat(format);

    GL_CHECK(glBindImageTexture(GLuint(unit), mTexture, GLint(level), GLboolean(layered), GLint(layer), access_, format_));
}
