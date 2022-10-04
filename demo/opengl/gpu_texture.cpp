#include <utility>
#include <cassert>
#include "stb_image.h"

#include "logger.h"
#include "filesystem.h"
#include "gpu_utils.h"
#include "gpu_texture.h"

GpuTexture::~GpuTexture() noexcept
{
    if (mTexture > 0) glDeleteTextures(1, &mTexture);
}

GpuTexture::GpuTexture(GpuTexture&& moved) noexcept
{
    mTexture = moved.mTexture;
    moved.mTexture = 0;
}

GpuTexture& GpuTexture::operator=(GpuTexture&& moved) noexcept
{
    mTexture = moved.mTexture;

    moved.mTexture = 0;

    return *this;
}

GpuTexture& GpuTexture::withMinFilter(FilterMin p)
{
    GLint value;

    switch (p)
    {
        case FilterMin::LINEAR:                 value = GL_LINEAR; break;
        case FilterMin::NEAREST:                value = GL_NEAREST; break;
        case FilterMin::LINEAR_MIPMAP_LINEAR:   value = GL_LINEAR_MIPMAP_LINEAR; break;
        case FilterMin::LINEAR_MIPMAP_NEAREST:  value = GL_LINEAR_MIPMAP_NEAREST; break;
        case FilterMin::NEAREST_MIPMAP_NEAREST: value = GL_NEAREST_MIPMAP_NEAREST; break;
        case FilterMin::NEAREST_MIPMAP_LINEAR:  value = GL_NEAREST_MIPMAP_LINEAR; break;
        default: value = GL_LINEAR;
    }

    mIntParams.push_back(std::make_pair(GL_TEXTURE_MIN_FILTER, value));

    return *this;
}

GpuTexture& GpuTexture::withMagFilter(FilterMag p)
{
    GLint value;

    switch (p)
    {
    case FilterMag::LINEAR:                 value = GL_LINEAR; break;
    case FilterMag::NEAREST:                value = GL_NEAREST; break;
    default: value = GL_LINEAR;
    }

    mIntParams.push_back(std::make_pair(GL_TEXTURE_MAG_FILTER, value));

    return *this;
}

GpuTexture& GpuTexture::withWrapS(Wrap p)
{
    GLint value = GL_castTexWrap(p);
    mIntParams.push_back(std::make_pair(GL_TEXTURE_WRAP_S, value));
    
    return *this;
}

GpuTexture& GpuTexture::withWrapT(Wrap p)
{
    GLint value = GL_castTexWrap(p);
    mIntParams.push_back(std::make_pair(GL_TEXTURE_WRAP_T, value));

    return *this;
}

GpuTexture& GpuTexture::withWrapR(Wrap p)
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
    if (getTarget() == TextureShape::TEX_CUBE_MAP) {
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
    if (getTarget() == TextureShape::TEX_CUBE_MAP) {
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
    if (getTarget() == TextureShape::TEX_CUBE_MAP) {
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
    if (getTarget() == TextureShape::TEX_CUBE_MAP) {
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

void GpuTexture::getDimensions(unsigned int& w, unsigned int& h)
{
    GLint width, height;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

    w = width;
    h = height;
}

GpuTexture2D::~GpuTexture2D()
{
    if (mTexture != INVALID_TEXTURE)
        GL_CHECK(glDeleteTextures(1, &mTexture));
}

GpuTexture2D::GpuTexture2D(GpuTexture2D&& moved) noexcept
{
    mTexture = moved.mTexture;

    moved.mTexture = 0;
}

GpuTexture2D& GpuTexture2D::operator=(GpuTexture2D&& moved) noexcept
{
    mTexture = moved.mTexture;

    moved.mTexture = 0;

    return *this;
}

bool GpuTexture2D::create(int w, int h, int level, InternalFormat internalFormat, InputFormat format, ComponentType type, const void* data)
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
    auto buffer = g_fileSystem.read_binary_file(fromFile);

    return createFromMemory(buffer.data(), buffer.size(), srgb, autoMipmap, compress);
}

bool GpuTexture2D::createFromMemory(const void* data, size_t bufLen, bool srgb, bool autoMipmap, bool compress)
{
    int x = 0, y = 0, n = 0;

    bool ok = stbi_info_from_memory((const uchar*)data, bufLen, &x, &y, &n);

    if (!ok) return false;

    if (!mTexture)
    {
        glGenTextures(1, &mTexture);
    }

    glBindTexture(GL_TEXTURE_2D, mTexture);

    GLenum format = GL_RGB;
    GLint internalFormat = GL_RGBA;
    if (srgb && compress) {
        internalFormat = GL_COMPRESSED_SRGB_ALPHA;
    }
    else if (srgb) {
        internalFormat = GL_SRGB8_ALPHA8;
    }
    else if (compress) {
        internalFormat = GL_COMPRESSED_RGBA;
    }

    switch (n)
    {
    case 1:
        format = GL_RED;
        break;
    case 2:
        format = GL_RG;
        break;
    case 3:
        format = GL_RGB;
        break;
    case 4:
        format = GL_RGBA;
        break;
    }

    stbi_set_flip_vertically_on_load(true);
    const uchar* img = stbi_load_from_memory((const uchar*)data, bufLen, &x, &y, &n, 0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D,
        0,
        internalFormat,
        x,
        y,
        0,
        format,
        GL_UNSIGNED_BYTE,
        img));

    if (autoMipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    return true;

}

bool GpuTexture2D::createRGB(int w, int h, int level)
{
    return createRGB8(w, h, level);
}

bool GpuTexture2D::createRG8U(int w, int h, int level)
{
    return create(w, h, level, InternalFormat::RG, InputFormat::RGB, ComponentType::UNSIGNED_BYTE, nullptr);
}

bool GpuTexture2D::createRG8S(int w, int h, int level)
{
    return create(w, h, level, InternalFormat::RG, InputFormat::RGB, ComponentType::BYTE, nullptr);
}

bool GpuTexture2D::createRG16U(int w, int h, int level)
{
    return create(w, h, level, InternalFormat::RG16, InputFormat::RGB, ComponentType::UNSIGNED_SHORT, nullptr);
}

bool GpuTexture2D::createRG16S(int w, int h, int level)
{
    return create(w, h, level, InternalFormat::RG16, InputFormat::RGB, ComponentType::SHORT, nullptr);
}

bool GpuTexture2D::createRG16F(int w, int h, int level)
{
    return create(w, h, level, InternalFormat::RG16F, InputFormat::RGB, ComponentType::FLOAT, nullptr);
}

bool GpuTexture2D::createRGB8(int w, int h, int level)
{
    return create(w, h, level, InternalFormat::RGBA, InputFormat::RGB, ComponentType::UNSIGNED_BYTE, nullptr);
}

bool GpuTexture2D::createRGB8S(int w, int h, int level)
{
    return create(w, h, level, InternalFormat::RGBA, InputFormat::RGB, ComponentType::BYTE, nullptr);
}

bool GpuTexture2D::createRGB32F(int w, int h, int level)
{
    return create(w, h, level, InternalFormat::RGBA32F, InputFormat::RGB, ComponentType::FLOAT, nullptr);
}

bool GpuTexture2D::createRGB16F(int w, int h, int level)
{
    return create(w, h, level, InternalFormat::RGBA16F, InputFormat::RGB, ComponentType::FLOAT, nullptr);
}

bool GpuTexture2D::createRGB10A2(int w, int h, int level)
{
    return create(w, h, level, InternalFormat::RGB10A2, InputFormat::RGB, ComponentType::FLOAT, nullptr);
}

bool GpuTexture2D::createR11G11B10(int w, int h, int level)
{
    return create(w, h, level, InternalFormat::R11F_G11F_B10F, InputFormat::RGB, ComponentType::FLOAT, nullptr);
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
    GL_CHECK(glActiveTexture(GL_TEXTURE0 + unit));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, mTexture));
}

void GpuTexture2D::bindImage(int unit, int level, Access access, ImageFormat format)
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
    if (!mTexture)
    {
        GL_CHECK(glGenTextures(1, &mTexture));
    }

    GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture));

    for (uint i = 0; i < fromFile.size(); ++i)
    {
        if (!cubemapHelper(fromFile[i], i, srgb, autoMipmap, compress)) {
            GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
            GL_CHECK(glDeleteTextures(1, &mTexture));
            mTexture = INVALID_TEXTURE;

            return false;
        }
    }

//    GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));

    return true;
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

void GpuTextureCubeMap::bindImage(int unit, int level, bool layered, int layer, Access access, ImageFormat format)
{
    GLenum access_ = GL_castImageAccess(access);
    GLenum format_ = GL_castImageFormat(format);

    GL_CHECK(glBindImageTexture(GLuint(unit), mTexture, GLint(level), GLboolean(layered), GLint(layer), access_, format_));
}

bool GpuTextureCubeMap::cubemapHelper(const std::string& fileName, uint index, bool srgb, bool autoMipmap, bool compress)
{
    int x = 0, y = 0, n = 0;

    bool ok = stbi_info(fileName.c_str(), &x, &y, &n);

    if (!ok) return false;

    GLenum format = GL_RGB;
    GLint internalFormat = GL_RGB;
    if (srgb && compress) {
        internalFormat = GL_COMPRESSED_SRGB;
    }
    else if (srgb) {
        internalFormat = GL_SRGB;
    }
    else if (compress) {
        internalFormat = GL_COMPRESSED_RGB;
    }

    switch (n)
    {
    case 1:
        format = GL_RED;
        break;
    case 2:
        format = GL_RG;
        break;
    case 3:
        format = GL_RGB;
        break;
    case 4:
        format = GL_RGBA;
        break;
    }

    stbi_set_flip_vertically_on_load(false);
    const uchar* img = stbi_load(fileName.c_str(), &x, &y, &n, 0);

    if (img == nullptr) return false;

    GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index,
        0,
        internalFormat,
        x,
        y,
        0,
        format,
        GL_UNSIGNED_BYTE,
        img));

    stbi_image_free((void*)img);

    if (autoMipmap) {
        GL_CHECK(glGenerateMipmap(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index));
    }

    return true;
}
