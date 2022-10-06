#include <utility>
#include <cassert>
#include "stb_image.h"

#include "logger.h"
#include "filesystem.h"
#include "gpu.h"
#include "gpu_utils.h"
#include "gpu_texture.h"

GpuTexture::~GpuTexture() noexcept
{
    GPU::deleteTexture(*this);
}

/*
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
*/

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

    _intParams.push_back(std::make_pair(GL_TEXTURE_MIN_FILTER, value));

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

    _intParams.push_back(std::make_pair(GL_TEXTURE_MAG_FILTER, value));

    return *this;
}

GpuTexture& GpuTexture::withWrapS(Wrap p)
{
    GLint value = GL_castTexWrap(p);
    _intParams.push_back(std::make_pair(GL_TEXTURE_WRAP_S, value));
    
    return *this;
}

GpuTexture& GpuTexture::withWrapT(Wrap p)
{
    GLint value = GL_castTexWrap(p);
    _intParams.push_back(std::make_pair(GL_TEXTURE_WRAP_T, value));

    return *this;
}

GpuTexture& GpuTexture::withWrapR(Wrap p)
{
    GLint value = GL_castTexWrap(p);
    _intParams.push_back(std::make_pair(GL_TEXTURE_WRAP_R, value));

    return *this;
}

GpuTexture& GpuTexture::withDefaultLinearRepeat()
{
    _intParams.push_back(std::make_pair(GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    _intParams.push_back(std::make_pair(GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    _intParams.push_back(std::make_pair(GL_TEXTURE_WRAP_S, GL_REPEAT));
    _intParams.push_back(std::make_pair(GL_TEXTURE_WRAP_T, GL_REPEAT));
    if (getTarget() == TextureShape::CUBE_MAP) {
        _intParams.push_back(std::make_pair(GL_TEXTURE_WRAP_R, GL_REPEAT));
    }

    return *this;
}

GpuTexture& GpuTexture::withDefaultLinearClampEdge()
{
    _intParams.push_back(std::make_pair(GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    _intParams.push_back(std::make_pair(GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    _intParams.push_back(std::make_pair(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    _intParams.push_back(std::make_pair(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    if (getTarget() == TextureShape::CUBE_MAP) {
        _intParams.push_back(std::make_pair(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    }

    return *this;
}

GpuTexture& GpuTexture::withDefaultMipmapRepeat()
{
    _intParams.push_back(std::make_pair(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    _intParams.push_back(std::make_pair(GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    _intParams.push_back(std::make_pair(GL_TEXTURE_WRAP_S, GL_REPEAT));
    _intParams.push_back(std::make_pair(GL_TEXTURE_WRAP_T, GL_REPEAT));
    if (getTarget() == TextureShape::CUBE_MAP) {
        _intParams.push_back(std::make_pair(GL_TEXTURE_WRAP_R, GL_REPEAT));
    }

    return *this;
}

GpuTexture& GpuTexture::withDefaultMipmapClampEdge()
{
    _intParams.push_back(std::make_pair(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    _intParams.push_back(std::make_pair(GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    _intParams.push_back(std::make_pair(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    _intParams.push_back(std::make_pair(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    if (getTarget() == TextureShape::CUBE_MAP) {
        _intParams.push_back(std::make_pair(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    }

    return *this;
}

void GpuTexture::updateParameters()
{
    GPU::updateTextureParameters(*this);

    _intParams.clear();
    _floatParams.clear();
}

void GpuTexture::selectUnit(int unit) const
{
    GPU::activeTexture(unit);
}

void GpuTexture::generateMipMaps() const
{
    GPU::generateMipmap(*this);
}

GpuTexture2D::~GpuTexture2D()
{
    GPU::deleteTexture(*this);
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
    GPU::createTexture2D(*this, w, h, level, internalFormat, format, type, data);

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

    bool ok = stbi_info_from_memory((const uchar*)data, int(bufLen), &x, &y, &n);

    if (!ok) return false;

    //GPU::bind(*this);

    InputFormat format = InputFormat::RGB;
    InternalFormat internalFormat = InternalFormat::RGBA;

    getTextureFormats(n, srgb, compress, internalFormat, format);

    stbi_set_flip_vertically_on_load(true);
    const uchar* img = stbi_load_from_memory((const uchar*)data, int(bufLen), &x, &y, &n, 0);

    GPU::createTexture2D(*this, x, y, 0, internalFormat, format, ComponentType::UNSIGNED_BYTE, img);

    if (autoMipmap) {
        GPU::generateMipmap(*this);
    }

    return true;

}

bool GpuTexture2D::createRGB(int w, int h, int level)
{
    return createRGB8(w, h, level);
}

bool GpuTexture2D::createRGB565(int w, int h, int level)
{
    return create(w, h, level, InternalFormat::RGB565, InputFormat::RGB, ComponentType::UNSIGNED_SHORT, nullptr);
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

std::shared_ptr<GpuTexture2D> GpuTexture2D::createShared()
{
    return std::make_shared<GpuTexture2D>();
}

GpuTextureCubeMap::~GpuTextureCubeMap()
{
    GPU::deleteTexture(*this);
}


bool GpuTextureCubeMap::createFromImage(const std::vector<std::string>& fromFile, bool srgb, bool autoMipmap, bool compress)
{

    for (uint i = 0; i < fromFile.size(); ++i)
    {
        if (!cubemapHelper(fromFile[i], i, srgb, autoMipmap, compress)) {
            GPU::resetTextureCube();
            GPU::deleteTexture(*this);
            mTexture = INVALID_TEXTURE;

            return false;
        }
    }


    return true;
}

bool GpuTextureCubeMap::cubemapHelper(const std::string& fileName, uint index, bool srgb, bool autoMipmap, bool compress)
{
    int x = 0, y = 0, n = 0;

    bool ok = stbi_info(fileName.c_str(), &x, &y, &n);

    if (!ok) return false;

    InputFormat format = InputFormat::RGB;
    InternalFormat internalFormat = InternalFormat::RGBA;

    getTextureFormats(n, srgb, compress, internalFormat, format);

    stbi_set_flip_vertically_on_load(false);
    const uchar* img = stbi_load(fileName.c_str(), &x, &y, &n, 0);

    if (img == nullptr) return false;

    GPU::createTextureCubeface(*this, x, y, 0, index, internalFormat, format, ComponentType::UNSIGNED_BYTE, img);
    stbi_image_free((void*)img);

    if (autoMipmap) {
        GPU::generateMipmapCubeface(*this, index);
    }

    return true;
}

void GpuTexture::getTextureFormats(int channels, bool srgb, bool compress, InternalFormat& internalFormat, InputFormat& format)
{
    format = InputFormat::RGB;
    internalFormat = InternalFormat::RGBA;

    if (srgb && compress) {
        internalFormat = InternalFormat::COMPRESSED_SRGB;
    }
    else if (srgb) {
        internalFormat = InternalFormat::SRGB_A;
    }
    else if (compress) {
        internalFormat = InternalFormat::COMPRESSED_RGBA;
    }

    switch (channels)
    {
    case 1:
        format = InputFormat::R;
        break;
    case 2:
        format = InputFormat::RG;
        break;
    case 3:
        format = InputFormat::RGB;
        break;
    case 4:
        format = InputFormat::RGBA;
        break;
    }
}
