#pragma once

#include <GL/glew.h>

#include "gpu_types.h"

extern void CheckOpenGLError(const char* stmt, const char* fname, int line);
extern GLenum GL_castDataType(eDataType type);
extern GLenum GL_castPixelFormat(ePixelFormat pf);
extern GLint GL_castTextureFormat(eTextureFormat f);
extern GLenum GL_castShaderStage(eShaderStage type);
extern GLint GL_castTexWrap(eTexWrap p);
extern GLenum GL_castImageAccess(eImageAccess p);
extern GLenum GL_castImageFormat(eImageFormat p);

extern const char* GetShaderStageTitle(eShaderStage type);

extern void GLAPIENTRY DebugMessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam);

#ifdef _DEBUG
#define GL_FLUSH_ERRORS while(glGetError() != GL_NO_ERROR) {}
#define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#define GL_FLUSH_ERRORS
#endif
