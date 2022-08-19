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

#ifdef _DEBUG
#define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#endif
