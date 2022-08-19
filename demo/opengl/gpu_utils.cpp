#include <GL/glew.h>
#include <SDL.h>
#include "gpu_types.h"
#include "gpu_utils.h"

void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        SDL_Log("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
        abort();
    }
}

GLenum GL_castDataType(eDataType type)
{
    switch (type) {
    case eDataType::BYTE:                return GL_BYTE;
    case eDataType::UNSIGNED_BYTE:       return GL_UNSIGNED_BYTE;
    case eDataType::SHORT:               return GL_SHORT;
    case eDataType::UNSIGNED_SHORT:      return GL_UNSIGNED_SHORT;
    case eDataType::INT32:               return GL_INT;
    case eDataType::UNSIGNED_INT32:      return GL_UNSIGNED_INT;
    case eDataType::FLOAT:               return GL_FLOAT;
    case eDataType::HALF_FLOAT:          return GL_HALF_FLOAT;
    case eDataType::UNSIGNED_INT_24_8:   return GL_UNSIGNED_INT_24_8;
    }
}

GLenum GL_castPixelFormat(ePixelFormat pf)
{
    using PF = ePixelFormat;

    switch (pf)
    {
    case PF::RGB:
        return GL_RGB;
    case PF::RGBA:
        return GL_RGBA;
    case PF::RGB8:
        return GL_RGB8;
    case PF::RGBA8:
        return GL_RGBA8;
    case PF::RGBA16:
        return GL_RGBA16;
    case PF::RGBA16F:
        return GL_RGBA16F;
    }
}

GLint GL_castTextureFormat(eTextureFormat f)
{
    switch (f)
    {
    case eTextureFormat::DEPTH24_STENCIL_8:
        return GL_DEPTH24_STENCIL8;
    case eTextureFormat::RGB:
        return GL_RGB;
    case eTextureFormat::RGBA:
        return GL_RGBA;
    case eTextureFormat::RGBA16F:
        return GL_RGBA16F;
    case eTextureFormat::RGBA32F:
        return GL_RGBA32F;
    case eTextureFormat::SRGB:
        return GL_SRGB;
    }
}

GLenum GL_castShaderStage(eShaderStage type)
{
    switch (type)
    {
    case eShaderStage::COMPUTE:
        return GL_COMPUTE_SHADER;
    case eShaderStage::VERTEX:
        return GL_VERTEX_SHADER;
    case eShaderStage::GEOMETRY:
        return GL_GEOMETRY_SHADER;
    case eShaderStage::FRAGMENT:
        return GL_FRAGMENT_SHADER;
    case eShaderStage::TESS_CONTROL:
        return GL_TESS_CONTROL_SHADER;
    case eShaderStage::TESS_EVALUATION:
        return GL_TESS_EVALUATION_SHADER;
    }
}

GLint GL_castTexWrap(eTexWrap p)
{
    switch (p)
    {
        case eTexWrap::CLAMP_TO_BORDER:     return GL_CLAMP_TO_BORDER;
        case eTexWrap::CLAMP_TO_EDGE:       return GL_CLAMP_TO_EDGE;
        case eTexWrap::MIRRORED_REPEAT:     return GL_MIRRORED_REPEAT;
        case eTexWrap::MIRROR_CLAMP_TO_EDGE:return GL_MIRROR_CLAMP_TO_EDGE;
        case eTexWrap::REPEAT:              return GL_REPEAT;
    }
}

const char* GetShaderStageTitle(eShaderStage type)
{
    switch (type)
    {
    case eShaderStage::COMPUTE:
        return "Compute";
    case eShaderStage::VERTEX:
        return "Vertex";
    case eShaderStage::GEOMETRY:
        return "Geometry";
    case eShaderStage::FRAGMENT:
        return "Fragment";;
    case eShaderStage::TESS_CONTROL:
        return "Tesselation control";
    case eShaderStage::TESS_EVALUATION:
        return "Tesselation evaluation";
    }
}

