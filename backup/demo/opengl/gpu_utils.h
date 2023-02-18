#pragma once

#include <GL/glew.h>

#include "gpu_types.h"

extern void CheckOpenGLError(const char* stmt, const char* fname, int line);
extern GLenum GL_castDataType(ComponentType type);
extern GLenum GL_castPixelFormat(InputFormat pf);
extern GLint GL_castTextureFormat(InternalFormat f);
extern GLenum GL_castShaderStage(ShaderType type);
extern GLint GL_castTexWrap(Wrap p);
extern GLenum GL_castImageAccess(Access p);
extern GLenum GL_castImageFormat(ImageFormat p);
extern GLenum GL_castDrawMode(eDrawMode p);
extern GLbitfield GL_castBufferAccessFlags(unsigned int p);
extern GLenum GL_CastBufferType(BufferTarget type);

extern const char* GetShaderStageTitle(ShaderType type);

extern void GLAPIENTRY DebugMessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam);

#ifdef _DEBUG
#define GL_FLUSH_ERRORS() while(glGetError() != GL_NO_ERROR) {}
#define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#define GL_FLUSH_ERRORS()
#endif
