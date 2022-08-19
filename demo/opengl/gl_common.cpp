#include <GL/glew.h>
#include <SDL.h>
#include "gl_common.h"

void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        SDL_Log("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
        abort();
    }
}

GLenum GL_CastDataType(eDataType type)
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

GLenum GL_CastPixelFormat(ePixelFormat pf)
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
