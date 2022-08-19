#include <utility>
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
}

bool GpuTexture2D::create(int w, int h, int level, eTextureFormat internalFormat, ePixelFormat format, eDataType type, const void** data)
{
    if (mTexture == INVALID_TEXTURE) GL_CHECK(glGenTextures(1, &mTexture));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, mTexture));
    
    const GLenum dataType = GL_castDataType(type);
    const GLenum pixFormat = GL_castPixelFormat(format);
    const GLint texFormat = GL_castTextureFormat(internalFormat);

    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, level, texFormat, w, h, 0, pixFormat, dataType, *data));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

    return true;
}

void GpuTexture2D::bind() const
{
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, mTexture));
}

