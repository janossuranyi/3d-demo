#pragma once

#include <set>
#include <SDL.h>
#include "engine2/Image.h"
#include "engine2/RenderCommon.h"

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

	inline GLenum GL_map_topology(eTopology x)
	{
		switch (x)
		{
		case TP_POINTS:	return GL_POINTS;
		case TP_LINES: return GL_LINES;
		case TP_LINE_STRIPS: return GL_LINE_STRIP;
		case TP_TRIANGLES: return GL_TRIANGLES;
		case TP_TRIANGLE_FANS: return GL_TRIANGLE_FAN;
		case TP_TRIANGLE_STRIPS: return GL_TRIANGLE_STRIP;
		default:
			assert(false);
		}
	}

	inline GLenum GL_map_texfilter(eImageFilter x)
	{
		switch (x)
		{
		case IFL_NEAREST:			return GL_NEAREST;
		case IFL_LINEAR:			return GL_LINEAR;
		case IFL_NEAREST_NEAREST:	return GL_NEAREST_MIPMAP_NEAREST;
		case IFL_NEAREST_LINEAR:	return GL_NEAREST_MIPMAP_LINEAR;
		case IFL_LINEAR_NEAREST:	return GL_LINEAR_MIPMAP_NEAREST;
		case IFL_LINEAR_LINEAR:		return GL_LINEAR_MIPMAP_LINEAR;
		default:
			assert(false);
		}
	}

	inline GLenum GL_map_texrepeat(eImageRepeat x)
	{
		switch (x)
		{
		case IMR_REPEAT:			return GL_REPEAT;
		case IMR_CLAMP_TO_BORDER:	return GL_CLAMP_TO_BORDER;
		case IMR_CLAMP_TO_EDGE:		return GL_CLAMP_TO_EDGE;
		default:
			assert(false);
		}
	}
	inline GLenum GL_map_textarget(eImageShape x)
	{
		switch (x)
		{
		case IMS_2D:			return GL_TEXTURE_2D;
		case IMS_2D_ARRAY:		return GL_TEXTURE_2D_ARRAY;
		case IMS_CUBEMAP:		return GL_TEXTURE_CUBE_MAP;
		case IMS_CUBEMAP_ARRAY:	return GL_TEXTURE_CUBE_MAP_ARRAY;
		default:
			assert(false);
		}
	}
	inline GLenum GL_map_blendFunc(eBlendFunc x)
	{
		switch (x)
		{
		case BFUNC_ZERO: return GL_ZERO;
		case BFUNC_ONE: return GL_ONE;
		case BFUNC_SRC_ALPHA: return GL_SRC_ALPHA;
		case BFUNC_ONE_MINUS_DST_COLOR: return GL_ONE_MINUS_DST_COLOR;
		case BFUNC_ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
		case BFUNC_SRC_COLOR: return GL_SRC_COLOR;
		case BFUNC_DST_ALPHA: return GL_DST_ALPHA;
		case BFUNC_DST_COLOR: return GL_DST_COLOR;
		case BFUNC_ONE_MINUS_DST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
		case BFUNC_ONE_MINUS_SRC_COLOR: return GL_ONE_MINUS_SRC_COLOR;
		default:
			assert(false);
		}
	}
	inline GLenum GL_map_blendEq(eBlendOp x)
	{
		switch (x)
		{
		case BOP_MAX:	return GL_MAX;
		case BOP_MIN:	return GL_MIN;
		case BOP_RSUB:	return GL_FUNC_REVERSE_SUBTRACT;
		case BOP_SUB:	return GL_FUNC_SUBTRACT;
		case BOP_ADD:	return GL_FUNC_ADD;
		default:
			assert(false);
		}
	}
	inline GLenum GL_map_stencilOp(eStencilOp x)
	{
		switch (x)
		{
		case SO_DEC:		return GL_DECR;
		case SO_DEC_WRAP:	return GL_DECR_WRAP;
		case SO_INC:		return GL_INCR;
		case SO_INC_WRAP:	return GL_INCR_WRAP;
		case SO_KEEP:		return GL_KEEP;
		case SO_REPLACE:	return GL_REPLACE;
		case SO_ZERO:		return GL_ZERO;
		case SO_INVERT:		return GL_INVERT;
		default:
			assert(false);
		}
	}
	inline GLenum GL_map_CmpOp(eCompOp x)
	{
		switch (x)
		{
		case CMP_ALWAYS: return GL_ALWAYS;
			break;
		case CMP_NEVER: return GL_NEVER;
			break;
		case CMP_GEQ: return GL_GEQUAL;
			break;
		case CMP_EQ: return GL_EQUAL;
			break;
		case CMP_NOTEQ: return GL_NOTEQUAL;
			break;
		case CMP_GT: return GL_GREATER;
			break;
		case CMP_LEQ: return GL_LEQUAL;
			break;
		case CMP_LT: return GL_LESS;
			break;
		default:
			assert(false);
		}
	}

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

