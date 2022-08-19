#pragma once

enum class eDataType { BYTE, UNSIGNED_BYTE, SHORT, UNSIGNED_SHORT, INT32, UNSIGNED_INT32, FLOAT, HALF_FLOAT, UNSIGNED_INT_24_8};
enum class ePixelFormat { RGB, RGBA, RGB8, RGBA8, RGBA16, RGBA16F, RGBA32F};

extern void CheckOpenGLError(const char* stmt, const char* fname, int line);
extern GLenum GL_CastDataType(eDataType type);
extern GLenum GL_CastPixelFormat(ePixelFormat pf);


#ifdef _DEBUG
#define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#endif
