#include "engine.h"
#include "logger.h"
#include <SDL.h>
#include <GL/glew.h>
#include <vector>
#include <set>

#ifdef _DEBUG
#define GL_FLUSH_ERRORS() while(glGetError() != GL_NO_ERROR) {}
#define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)

#define GLC() do { \
            CheckOpenGLError("::", __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#define GL_FLUSH_ERRORS()
#define GLC()
#endif

static void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		Error("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
		abort();
	}
}

void GLAPIENTRY _DebugMessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const GLvoid* userParam);

namespace jsr {

	struct TextureFormatInfo {
		GLenum internal_format;
		GLenum internal_format_srgb;
		GLenum format;
		GLenum type;
		bool normalized;
		uint16_t componentCount;
	};

	static TextureFormatInfo s_texture_format[] = {
		{GL_ALPHA,              GL_ZERO,         GL_ALPHA,            GL_UNSIGNED_BYTE,                true,  1}, // A8
		{GL_R8,                 GL_ZERO,         GL_RED,              GL_UNSIGNED_BYTE,                true,  1}, // R8
		{GL_R8I,                GL_ZERO,         GL_RED,              GL_BYTE,                         true,  1}, // R8I
		{GL_R8UI,               GL_ZERO,         GL_RED,              GL_UNSIGNED_BYTE,                true,  1}, // R8U
		{GL_R8_SNORM,           GL_ZERO,         GL_RED,              GL_BYTE,                         true,  1}, // R8S
		{GL_R16,                GL_ZERO,         GL_RED,              GL_UNSIGNED_SHORT,               true,  1}, // R16
		{GL_R16I,               GL_ZERO,         GL_RED,              GL_SHORT,                        true,  1}, // R16I
		{GL_R16UI,              GL_ZERO,         GL_RED,              GL_UNSIGNED_SHORT,               true,  1}, // R16U
		{GL_R16F,               GL_ZERO,         GL_RED,              GL_HALF_FLOAT,                   false, 1}, // R16F
		{GL_R16_SNORM,          GL_ZERO,         GL_RED,              GL_SHORT,                        true,  1}, // R16S
		{GL_R32I,               GL_ZERO,         GL_RED,              GL_INT,                          true,  1}, // R32I
		{GL_R32UI,              GL_ZERO,         GL_RED,              GL_UNSIGNED_INT,                 true,  1}, // R32U
		{GL_R32F,               GL_ZERO,         GL_RED,              GL_FLOAT,                        false, 1}, // R32F
		{GL_RG8,                GL_ZERO,         GL_RG,               GL_UNSIGNED_BYTE,                true,  2}, // RG8
		{GL_RG8I,               GL_ZERO,         GL_RG,               GL_BYTE,                         true,  2}, // RG8I
		{GL_RG8UI,              GL_ZERO,         GL_RG,               GL_UNSIGNED_BYTE,                true,  2}, // RG8U
		{GL_RG8_SNORM,          GL_ZERO,         GL_RG,               GL_BYTE,                         true,  2}, // RG8S
		{GL_RG16,               GL_ZERO,         GL_RG,               GL_UNSIGNED_SHORT,               true,  2}, // RG16
		{GL_RG16I,              GL_ZERO,         GL_RG,               GL_SHORT,                        true,  2}, // RG16I
		{GL_RG16UI,             GL_ZERO,         GL_RG,               GL_UNSIGNED_SHORT,               true,  2}, // RG16U
		{GL_RG16F,              GL_ZERO,         GL_RG,               GL_FLOAT,                        false, 2}, // RG16F
		{GL_RG16_SNORM,         GL_ZERO,         GL_RG,               GL_SHORT,                        true,  2}, // RG16S
		{GL_RG32I,              GL_ZERO,         GL_RG,               GL_INT,                          true,  2}, // RG32I
		{GL_RG32UI,             GL_ZERO,         GL_RG,               GL_UNSIGNED_INT,                 true,  2}, // RG32U
		{GL_RG32F,              GL_ZERO,         GL_RG,               GL_FLOAT,                        false, 2}, // RG32F
		{GL_RGB8,               GL_SRGB8,        GL_RGB,              GL_UNSIGNED_BYTE,                true,  3}, // RGB8
		{GL_RGB8I,              GL_ZERO,         GL_RGB,              GL_BYTE,                         true,  3}, // RGB8I
		{GL_RGB8UI,             GL_ZERO,         GL_RGB,              GL_UNSIGNED_BYTE,                true,  3}, // RGB8U
		{GL_RGB8_SNORM,         GL_ZERO,         GL_RGB,              GL_BYTE,                         true,  3}, // RGB8S
		{GL_RGB32F,             GL_ZERO,         GL_RGB,              GL_FLOAT,                        false, 3}, // RGB32F
		{GL_RGBA8,              GL_SRGB8_ALPHA8, GL_BGRA,             GL_UNSIGNED_BYTE,                true,  4}, // BGRA8
		{GL_RGBA8,              GL_SRGB8_ALPHA8, GL_RGBA,             GL_UNSIGNED_BYTE,                true,  4}, // RGBA8
		{GL_RGBA8I,             GL_ZERO,         GL_RGBA,             GL_BYTE,                         true,  4}, // RGBA8I
		{GL_RGBA8UI,            GL_ZERO,         GL_RGBA,             GL_UNSIGNED_BYTE,                true,  4}, // RGBA8U
		{GL_RGBA8_SNORM,        GL_ZERO,         GL_RGBA,             GL_BYTE,                         true,  4}, // RGBA8S
		{GL_RGBA16,             GL_ZERO,         GL_RGBA,             GL_UNSIGNED_SHORT,               true,  4}, // RGBA16
		{GL_RGBA16I,            GL_ZERO,         GL_RGBA,             GL_SHORT,                        true,  4}, // RGBA16I
		{GL_RGBA16UI,           GL_ZERO,         GL_RGBA,             GL_UNSIGNED_SHORT,               true,  4}, // RGBA16U
		{GL_RGBA16F,            GL_ZERO,         GL_RGBA,             GL_HALF_FLOAT,                   false, 4}, // RGBA16F
		{GL_RGBA16_SNORM,       GL_ZERO,         GL_RGBA,             GL_SHORT,                        true,  4}, // RGBA16S
		{GL_RGBA32I,            GL_ZERO,         GL_RGBA,             GL_INT,                          true,  4}, // RGBA32I
		{GL_RGBA32UI,           GL_ZERO,         GL_RGBA,             GL_UNSIGNED_INT,                 true,  4}, // RGBA32U
		{GL_RGBA32F,            GL_ZERO,         GL_RGBA,             GL_FLOAT,                        false, 4}, // RGBA32F
		{GL_RGBA4,              GL_ZERO,         GL_RGBA,             GL_UNSIGNED_SHORT_4_4_4_4_REV,   true,  4}, // RGBA4
		{GL_RGB5_A1,            GL_ZERO,         GL_RGBA,             GL_UNSIGNED_SHORT_1_5_5_5_REV,   true,  4}, // RGB5A1
		{GL_RGB10_A2,           GL_ZERO,         GL_RGBA,             GL_UNSIGNED_INT_2_10_10_10_REV,  true,  4}, // RGB10A2
		{GL_R11F_G11F_B10F,     GL_ZERO,         GL_RGB,              GL_UNSIGNED_INT_10F_11F_11F_REV, true,  3}, // RG11B10F
		{GL_COMPRESSED_RGB,     GL_COMPRESSED_SRGB,GL_RGB,            GL_UNSIGNED_BYTE,                true,  3}, // RGB8_COMPRESSED
		{GL_COMPRESSED_RGBA,    GL_COMPRESSED_SRGB_ALPHA,GL_RGBA,     GL_UNSIGNED_BYTE,                true,  4}, // RGBA8_COMPRESSED

		{GL_COMPRESSED_RGB_S3TC_DXT1_EXT,	GL_COMPRESSED_SRGB_S3TC_DXT1_EXT, GL_RGB, GL_UNSIGNED_BYTE,true,  3}, // RGB_DXT1
		{GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,	GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, GL_RGBA, GL_UNSIGNED_BYTE, true, 4}, // RGBA_DXT1
		{GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,	GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, GL_RGBA, GL_UNSIGNED_BYTE, true, 4}, // RGBA_DXT3
		{GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,	GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, GL_RGBA, GL_UNSIGNED_BYTE, true, 4}, // RGBA_DXT5
		{GL_COMPRESSED_RGBA_BPTC_UNORM,		GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM,	GL_RGBA, GL_UNSIGNED_BYTE, true, 4}, // RGBA_BPTC aka. BC7

		{GL_DEPTH_COMPONENT16,  GL_ZERO,         GL_DEPTH_COMPONENT,  GL_UNSIGNED_SHORT,               true,  1}, // D16
		{GL_DEPTH_COMPONENT24,  GL_ZERO,         GL_DEPTH_COMPONENT,  GL_UNSIGNED_INT,                 true,  1}, // D24
		{GL_DEPTH24_STENCIL8,   GL_ZERO,         GL_DEPTH_STENCIL,    GL_UNSIGNED_INT_24_8,            true,  1}, // D24S8
		{GL_DEPTH_COMPONENT32,  GL_ZERO,         GL_DEPTH_COMPONENT,  GL_UNSIGNED_INT,                 true,  1}, // D32
		{GL_DEPTH_COMPONENT32F, GL_ZERO,         GL_DEPTH_COMPONENT,  GL_FLOAT,                        false, 1}, // D16F
		{GL_DEPTH_COMPONENT32F, GL_ZERO,         GL_DEPTH_COMPONENT,  GL_FLOAT,                        false, 1}, // D24F
		{GL_DEPTH_COMPONENT32F, GL_ZERO,         GL_DEPTH_COMPONENT,  GL_FLOAT,                        false, 1}, // D32F
		{GL_STENCIL_INDEX8,     GL_ZERO,         GL_STENCIL_INDEX,    GL_UNSIGNED_BYTE,                true,  1}, // D0S8
	};

	struct oglContext {
		SDL_Window* wnd;
		SDL_GLContext ctx;
		GpuCapabilities deviceCapabilities;
		std::set<std::string> extensions;
		int version;
		const char* renderer;
		const char* rendererVersion;
		std::vector<GLuint> unused_buffers;
		std::vector<GLuint> unused_shaders;
		bool initialzed;
		int width;
		int height;
	};

	struct buffer_t {
		oglContext * ctx;
		GLenum	target;
		GLenum	mapflags;
		GLuint	buffer;
		void*	mappedPtr;
		int		size;
		~buffer_t() { ctx->unused_buffers.push_back(buffer);}
	};

	struct shader_t {
		oglContext * ctx;
		GLenum	stage;
		GLuint shader;
		~shader_t() { ctx->unused_shaders.push_back(shader); }
	};

	struct grPipeline_t {
		oglContext* ctx;
		GLuint vao;

	};

	GfxCore::GfxCore()
	{
		pInternals = new oglContext{};
	}

	GfxCore::~GfxCore()
	{
		delete pInternals;
	}

	Result GfxCore::CreateVertexBuffer(int size, GpuBufferHandle& handle)
	{
		buffer_t* hnd = new buffer_t{};
		hnd->mapflags = GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT;
		hnd->size = size;
		hnd->target = GL_ARRAY_BUFFER;
		hnd->ctx = static_cast<oglContext*>(pInternals);

		GL_CHECK(glCreateBuffers(1, &hnd->buffer));
		GL_CHECK(glNamedBufferStorage(hnd->buffer, size, nullptr, hnd->mapflags));

		handle.reset(hnd);

		return Result::success;
	}

	Result GfxCore::CmdUpdateBuffer(GpuBufferHandle handle, SharedPtr data, int offset, int size)
	{
		const buffer_t* hnd = static_cast<buffer_t*>(handle.get());
		GL_CHECK(glNamedBufferSubData(hnd->buffer, offset, size, data.get()));

		return Result::success;
	}

	Result GfxCore::CreateIndexBuffer(int size, GpuBufferHandle& handle)
	{
		buffer_t* hnd = new buffer_t{};
		hnd->mapflags = GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT;
		hnd->size = size;
		hnd->target = GL_ELEMENT_ARRAY_BUFFER;
		hnd->ctx = static_cast<oglContext*>(pInternals);

		GL_CHECK(glCreateBuffers(1, &hnd->buffer));
		GL_CHECK(glNamedBufferStorage(hnd->buffer, size, nullptr, hnd->mapflags));

		handle.reset(hnd);

		return Result::success;
	}

	Result GfxCore::CreateUniformBuffer(int size, GpuBufferHandle& handle)
	{
		buffer_t* hnd = new buffer_t{};
		hnd->mapflags = GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT;
		hnd->size = size;
		hnd->target = GL_UNIFORM_BUFFER;
		hnd->ctx = static_cast<oglContext*>(pInternals);

		GL_CHECK(glCreateBuffers(1, &hnd->buffer));
		GL_CHECK(glNamedBufferStorage(hnd->buffer, size, nullptr, hnd->mapflags));

		handle.reset(hnd);

		return Result::success;
	}

	Result GfxCore::CreateShader(const ShaderStageDef& def, GpuShaderHandle& handle)
	{
		oglContext* c = static_cast<oglContext*>(pInternals);
		shader_t* hnd = new shader_t();
		hnd->ctx = c;
		switch (def.stage) {
		case eShaderStage::vertex: hnd->stage = GL_VERTEX_SHADER; break;
		case eShaderStage::geometry: hnd->stage = GL_GEOMETRY_SHADER; break;
		case eShaderStage::fragment: hnd->stage = GL_FRAGMENT_SHADER; break;
		case eShaderStage::compute: hnd->stage = GL_COMPUTE_SHADER; break;
		}

		GLuint shader = 0xffff;
		GL_CHECK(shader = glCreateShader(hnd->stage));

		if (shader == 0xffff) {
			return Result::failed;
		}

		const GLchar* tmp = static_cast<const GLchar*>(def.code);
		GL_CHECK(glShaderSource(shader, 1, &tmp, nullptr));
		GLint result = GL_FALSE;

		GL_CHECK(glCompileShader(shader));
		GL_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &result));

		if (result == GL_FALSE)
		{
			GLint infologLen;
			GL_CHECK(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen));
			if (infologLen > 0) {
				std::vector<char> logBuf(infologLen);
				GL_CHECK(glGetShaderInfoLog(shader, infologLen, nullptr, logBuf.data()));
				Error("Shader compiler error: %s", logBuf.data());
			}

			delete hnd;
			GL_CHECK(glDeleteShader(shader));
			return Result::failed;
		}

		hnd->shader = shader;
		handle.reset(hnd);

		return Result::success;
	}

	Result GfxCore::CreateGraphicsPipeline(const GraphicsPipelineDef& def, GraphicsPipelineHandle& handle)
	{
		return Result();
	}

	Result GfxCore::CreateWindow(int width, int height, int fullscreen)
	{
		int err;

		auto* ctx = static_cast<oglContext*>(pInternals);


		if ((err = SDL_Init(SDL_INIT_VIDEO) < 0)) {
			Error("ERROR: %s", SDL_GetError());
			return Result::failed;
		}

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

#if _DEBUG
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);

		Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

		if (fullscreen == 2)
		{
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
		else if (fullscreen == 1)
		{
			flags |= SDL_WINDOW_FULLSCREEN;
		}

		Info("SDL_CreateWindow start");
		ctx->wnd = SDL_CreateWindow("GfxCoreGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
		if (!ctx->wnd)
		{
			Error("Cannot create window - %s", SDL_GetError());
			return Result::failed;
		}

		Info("SDL_CreateWindow done");

		ctx->ctx = SDL_GL_CreateContext(ctx->wnd);
		if (!ctx->ctx)
		{
			Error("Cannot create GL context - %s", SDL_GetError());
			return Result::failed;
		}
		// Initialize GLEW
		glewExperimental = true; // Needed for core profile
		if (glewInit() != GLEW_OK) {
			Error("Cannot initialize GLEW");
			return Result::failed;
		}

		Info("Using GLEW %s", glewGetString(GLEW_VERSION));

		SDL_GL_SetSwapInterval(1);

		std::string renderer = (char*)glGetString(GL_RENDERER);
		std::string version = (char*)glGetString(GL_VERSION);

		Info("OpenGL extensions");
		GLint numExts; glGetIntegerv(GL_NUM_EXTENSIONS, &numExts);
		std::string exts;

		for (int ext_ = 0; ext_ < numExts; ++ext_)
		{
			const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, ext_);
			//Info("%s", extension);
			ctx->extensions.emplace(extension);
			exts.append(extension).append(" ");
		}
		const float gl_version = float(atof(version.c_str()));
		ctx->version = int(gl_version * 100);

		if (ctx->version < 450)
		{
			Error("GL_VERSION < 4.5");
			return Result::failed;
		}

		Info("extensions: %s", exts.c_str());
		SDL_version ver;

		SDL_GetVersion(&ver);

		Info("GL Renderer: %s", renderer.c_str());
		Info("GL Version: %s (%.2f)", version.c_str(), gl_version);
		Info("SDL version: %d.%d patch %d", (int)ver.major, (int)ver.minor, (int)ver.patch);

		int _w, _h;

		SDL_GL_GetDrawableSize(ctx->wnd, &_w, &_h);
		glViewport(0, 0, _w, _h);
		glScissor(0, 0, _w, _h);
		ctx->width = _w;
		ctx->height = _h;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

#ifdef GFX_ENABLE_SRGB_FRAMEBUFFER
		glEnable(GL_FRAMEBUFFER_SRGB);
#endif
		ctx->renderer = (const char*)glGetString(GL_RENDERER);
		ctx->rendererVersion = (const char*)glGetString(GL_VERSION);
		glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &ctx->deviceCapabilities.maxArrayTextureLayers);

		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &ctx->deviceCapabilities.maxFragmentTextureImageUnits);
		glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &ctx->deviceCapabilities.maxVertexTextureImageUnits);
		glGetIntegerv(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, &ctx->deviceCapabilities.maxComputeTextureImageUnits);

		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &ctx->deviceCapabilities.maxTextureSize);
		glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &ctx->deviceCapabilities.maxComputeSharedMemorySize);
		glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &ctx->deviceCapabilities.maxUniformBlockSize);
		glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &ctx->deviceCapabilities.maxShaderStorageBlockSize);
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &ctx->deviceCapabilities.maxVertexAttribs);
		glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &ctx->deviceCapabilities.maxVertexAttribBindings);

		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &ctx->deviceCapabilities.uniformBufferOffsetAligment);
		glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &ctx->deviceCapabilities.availableVideoMemory);

		glGetError();
		glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, &ctx->deviceCapabilities.availableVideoMemory);
		glGetError();
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glEnable(GL_DEPTH_TEST);

#ifdef _DEBUG
		if (ctx->extensions.count("GL_ARB_debug_output"))
		{
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

			glDebugMessageCallbackARB(&_DebugMessageCallback, NULL);
#if 0
			glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
			glDebugMessageControlARB(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR_ARB, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			glDebugMessageControlARB(GL_DONT_CARE, GL_DEBUG_TYPE_PERFORMANCE_ARB, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			glDebugMessageControlARB(GL_DONT_CARE, GL_DEBUG_TYPE_PORTABILITY_ARB, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			glDebugMessageControlARB(GL_DONT_CARE, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB, GL_DONT_CARE, 0, nullptr, GL_TRUE);

			glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
			glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
#endif
		}
#endif
		ctx->initialzed = true;
		return Result::success;

	}

	bool GfxCore::IsInitialized() const
	{
		return static_cast<oglContext*>(pInternals)->initialzed;
	}

	void GfxCore::gc() const
	{
		oglContext* ctx = static_cast<oglContext*>(pInternals);
		if (ctx->unused_buffers.empty() == false)
		{
			GL_CHECK(glDeleteBuffers(ctx->unused_buffers.size(), ctx->unused_buffers.data()));
			ctx->unused_buffers.clear();
		}
	}

	void GfxCore::GetScreenSize(int& x, int& y) const
	{
		const oglContext* ctx = static_cast<oglContext*>(pInternals);
		x = ctx->width;
		y = ctx->height;
	}
}

static void GLAPIENTRY JSE_DebugMessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const GLvoid* userParam)
{
	// Convert enums into a humen readable text
	// See: https://www.opengl.org/registry/specs/ARB/debug_output.txt

//    if (severity > GL_DEBUG_SEVERITY_MEDIUM) return;

	const char* sourceText = "Unknown";
	switch (source)
	{
	case GL_DEBUG_SOURCE_API_ARB:
		// The GL
		sourceText = "API";
		break;
		// The GLSL shader compiler or compilers for other extension - provided languages
	case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
		sourceText = "Shader compiler";
		break;
		// The window system, such as WGL or GLX
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
		sourceText = "Window system";
		break;
		// External debuggers or third-party middleware libraries
	case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
		sourceText = "Third party";
		break;
		// The application
	case GL_DEBUG_SOURCE_APPLICATION_ARB:
		sourceText = "Application";
		break;
		// Sources that do not fit to any of the ones listed above
	case GL_DEBUG_SOURCE_OTHER_ARB:
		sourceText = "Other";
		break;
	}

	const char* typeText = "Unknown";
	switch (type)
	{
		// Events that generated an error
	case GL_DEBUG_TYPE_ERROR_ARB:
		typeText = "Error";
		break;
		// Behavior that has been marked for deprecation
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
		typeText = "Deprecated behavior";
		break;
		// Behavior that is undefined according to the specification
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
		typeText = "Undefined behavior";
		break;
		// Implementation-dependent performance warnings
	case GL_DEBUG_TYPE_PERFORMANCE_ARB:
		typeText = "Performance";
		break;
		// Use of extensions or shaders in a way that is highly vendor - specific
	case GL_DEBUG_TYPE_PORTABILITY_ARB:
		typeText = "Portability";
		break;
		// Types of events that do not fit any of the ones listed above
	case GL_DEBUG_TYPE_OTHER_ARB:
		typeText = "Other";
		break;
	}

	const char* severityText = "Unknown";
	switch (severity)
	{
		// Any GL error; dangerous undefined behavior; any GLSL or ARB shader compiler and linker errors;
	case GL_DEBUG_SEVERITY_HIGH_ARB:
		severityText = "High";
		break;
		// Severe performance warnings; GLSL or other shader compiler and linker warnings; use of currently deprecated behavior
	case GL_DEBUG_SEVERITY_MEDIUM_ARB:
		severityText = "Medium";
		break;
		// Performance warnings from redundant state changes; trivial undefined behavior
	case GL_DEBUG_SEVERITY_LOW_ARB:
		severityText = "Low";
		break;
	}

	// Unused params
	(void)id;
	(void)length;
	(void)userParam;

	// Replace LogDebug with your logging function
	Info("[OpenGL:source='%s', type='%s', severity='%s'] %s",
		sourceText,
		typeText,
		severityText,
		message);
}

