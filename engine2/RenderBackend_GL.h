#pragma once

#include "./Image.h"

namespace jsr {
    struct imageFormatInfo_t
    {
        GLenum internalFormat;
        GLenum internalFormatSRGB;
        GLenum format;
        GLenum type;
        bool normalized;
        int numComponent;
    };
    
    extern const imageFormatInfo_t s_image_formats[];

    GLenum GL_map_texfilter(eImageFilter x);
    GLenum GL_map_texrepeat(eImageRepeat x);
    GLenum GL_map_textarget(eImageShape x);

}

void CheckOpenGLError(const char* stmt, const char* fname, int line);

#ifdef _DEBUG
#define GL_FLUSH_ERRORS() while(glGetError() != GL_NO_ERROR) {}
#define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)

#define GLC() do { \
            CheckOpenGLError("[]", __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#define GL_FLUSH_ERRORS()
#define GLC()
#endif

