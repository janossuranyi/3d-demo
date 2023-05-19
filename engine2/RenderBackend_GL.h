#pragma once

#include <set>
#include <SDL.h>
#include "./Image.h"
#include "./RenderCommon.h"

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

	struct glconfig_t
	{
		int				resX;
		int				resY;
		SDL_Window*		hwnd;
		SDL_GLContext	glctx;
		int				msaa;
		int				vsync;
		int				version;
		int				maxFragmentTextureImageUnits;
		int				maxVertexTextureImageUnits;
		int				maxComputeTextureImageUnits;
		int				maxArrayTextureLayers;
		int				maxTextureSize;
		int				maxComputeSharedMemorySize;
		int				maxUniformBlockSize;
		int				maxShaderStorageBlockSize;
		int				maxVertexAttribs;
		int				maxVertexAttribBindings;
		int				uniformBufferOffsetAligment;
		int				availableVideoMemory;

		std::set<std::string> extensions;
		GLuint			currentVertexArray{ 0 };
	};

    extern const imageFormatInfo_t s_image_formats[];
	extern glconfig_t glconfig;

    GLenum GL_map_texfilter(eImageFilter x);
    GLenum GL_map_texrepeat(eImageRepeat x);
    GLenum GL_map_textarget(eImageShape x);
	GLenum GL_map_topology(eTopology x);
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

