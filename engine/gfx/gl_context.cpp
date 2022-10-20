#include <SDL.h>
#include <GL/glew.h>
#include <cassert>
#include <unordered_map>
#include "handle.h"
#include "gl_context.h"
#include "logger.h"


namespace gfx {

	static const char* MapShaderStageTitle(ShaderStage type)
	{
		switch (type)
		{
		case ShaderStage::Compute:
			return "Compute";
		case ShaderStage::Vertex:
			return "Vertex";
		case ShaderStage::Geometry:
			return "Geometry";
		case ShaderStage::Fragment:
			return "Fragment";
		}

		return "Unknown";
	}

	static void CheckOpenGLError(const char* stmt, const char* fname, int line)
	{
		GLenum err = glGetError();
		if (err != GL_NO_ERROR)
		{
			Error("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
			abort();
		}
	}

	static void GLAPIENTRY DebugMessageCallback(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar* message,
		const GLvoid* userParam);

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

	static inline GLenum MapBufferUsage(const BufferUsage usage)
	{
		switch (usage) {
		case BufferUsage::Static:
			return GL_STATIC_DRAW;
		case BufferUsage::Dynamic:
			return GL_DYNAMIC_DRAW;
		case BufferUsage::Stream:
			return GL_STREAM_DRAW;
		}

		return GL_STATIC_DRAW;
	};

	static inline GLenum MapShaderStage(const ShaderStage type)
	{
		switch (type) {
		case ShaderStage::Vertex:
			return GL_VERTEX_SHADER;
		case ShaderStage::Fragment:
			return GL_FRAGMENT_SHADER;
		case ShaderStage::Geometry:
			return GL_GEOMETRY_SHADER;
		case ShaderStage::Compute:
			return GL_COMPUTE_SHADER;
		}

		return GL_VERTEX_SHADER;
	}

	int OpenGLRenderContext::get_red_bits() const {
		return window_.redBits;
	}

	int OpenGLRenderContext::get_green_bits() const
	{
		return window_.greenBits;
	}

	int OpenGLRenderContext::get_blue_bits() const
	{
		return window_.blueBits;
	}

	int OpenGLRenderContext::get_depth_bits() const
	{
		return window_.depthBits;
	}

	int OpenGLRenderContext::get_stencil_bits() const
	{
		return window_.stencilBits;
	}

	void OpenGLRenderContext::operator()(const cmd::CreateVertexBuffer& cmd)
	{
		if (vertex_buffer_map_.count(cmd.handle) > 0)
			return;

		GLuint buffer = static_cast<GLuint>(0xffff);
		const GLenum _usage = MapBufferUsage(cmd.usage);

		GL_CHECK(glGenBuffers(1, &buffer));
		assert(buffer != 0xffff);

		const uint32_t _size = cmd.data.data() ? cmd.data.size() : cmd.size;

		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, buffer));
		if (cmd.data.data())
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(cmd.data.size()), cmd.data.data(), _usage));
		else
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(cmd.size), nullptr, _usage));

		vertex_buffer_map_.emplace(cmd.handle, VertexBufferData{ buffer, _size, cmd.usage });
	}

	void OpenGLRenderContext::operator()(const cmd::UpdateVertexBuffer& cmd)
	{
		if (cmd.data.data() == nullptr)
			return;

		const auto result = vertex_buffer_map_.find(cmd.handle);
		if (result == vertex_buffer_map_.end())
			return;

		const auto& data = result->second;
		const GLsizeiptr size = cmd.size > 0 ? GLsizeiptr(cmd.size) : cmd.data.size();
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, data.buffer));
		GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, GLintptr(cmd.offset), size, cmd.data.data()));
	}

	void OpenGLRenderContext::operator()(const cmd::DeleteVertexBuffer& cmd)
	{
		const auto result = vertex_buffer_map_.find(cmd.handle);
		if (result == vertex_buffer_map_.end())
			return;

		const auto& data = result->second;

		GL_CHECK(glDeleteBuffers(1, &data.buffer));

		vertex_buffer_map_.erase(cmd.handle);
	}

	void OpenGLRenderContext::operator()(const cmd::CreateIndexBuffer& cmd)
	{
		const auto result = index_buffer_map_.find(cmd.handle);
		if (result != index_buffer_map_.end())
			return;

		GLuint buffer = static_cast<GLuint>(0xffff);
		const GLenum _usage = MapBufferUsage(cmd.usage);

		GL_CHECK(glGenBuffers(1, &buffer));
		assert(buffer != 0xffff);

		const uint32_t _size = cmd.data.data() ? cmd.data.size() : cmd.size;

		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer));
		if (cmd.data.data())
			GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, GLsizeiptr(cmd.data.size()), cmd.data.data(), _usage));
		else
			GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, GLsizeiptr(cmd.size), nullptr, _usage));

		index_buffer_map_.emplace(cmd.handle, IndexBufferData{ buffer, _size, cmd.usage, cmd.type });

	}

	void OpenGLRenderContext::operator()(const cmd::UpdateIndexBuffer& cmd)
	{
		if (cmd.data.data() == nullptr)
			return;

		const auto result = index_buffer_map_.find(cmd.handle);
		if (result == index_buffer_map_.end())
			return;

		const auto& data = result->second;
		const GLsizeiptr size = data.size > 0 ? GLsizeiptr(data.size) : cmd.data.size();
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,		data.buffer));
		GL_CHECK(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,	GLintptr(cmd.offset), size, cmd.data.data()));

	}

	void OpenGLRenderContext::operator()(const cmd::DeleteIndexBuffer& cmd)
	{
		const auto result = index_buffer_map_.find(cmd.handle);
		if (result == index_buffer_map_.end())
			return;

		const auto& data = result->second;
		GL_CHECK(glDeleteBuffers(1, &data.buffer));
		index_buffer_map_.erase(cmd.handle);
	}

	void OpenGLRenderContext::operator()(const cmd::CreateShader& cmd)
	{
		if (shader_map_.count(cmd.handle) > 0)
			return;

		GLuint shader = 0xffff;
		GL_CHECK(shader = glCreateShader(MapShaderStage(cmd.stage)));
		
		assert(shader != 0xffff);

		const GLchar* tmp = cmd.source.c_str();
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
				const char* sType = MapShaderStageTitle(cmd.stage);
				Error("%s shader compilation failed: %s", sType, logBuf.data());
			}

			GL_CHECK(glDeleteShader(shader));
			return;
		}
		shader_map_.emplace(cmd.handle, ShaderData{ shader, cmd.source, cmd.stage, true });
	}

	void OpenGLRenderContext::operator()(const cmd::DeleteShader& cmd)
	{
		auto result = shader_map_.find(cmd.handle);
		if (result == shader_map_.end())
			return;

		auto& data = result->second;
		GL_CHECK(glDeleteShader(data.shader));

		shader_map_.erase(cmd.handle);
	}

	bool OpenGLRenderContext::create_window(uint16_t w, uint16_t h, bool fullscreen, const std::string& name) {
		int err;

		if ((err = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0)) {
			Error("ERROR: %s", SDL_GetError());
			return false;
		}

		Info("SD_Init done");

		/*
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		*/
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		//SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, SDL_TRUE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
		//SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, SDL_TRUE);

		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, awindow.multisample);

		Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

		if (fullscreen && (w == 0 || h == 0))
		{
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
		else if (fullscreen)
		{
			flags |= SDL_WINDOW_FULLSCREEN;
		}

		Info("SD_CreateWindow start");
		windowHandle_ = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, flags);
		if (!windowHandle_)
		{
			Error("Cannot create window - %s", SDL_GetError());
			return false;
		}

		Info("SD_CreateWindow done");

		glcontext_ = SDL_GL_CreateContext(windowHandle_);
		if (!glcontext_)
		{
			Error("Cannot create GL context - %s", SDL_GetError());
			return false;
		}

		Info("SDL_GL_CreateContext done");

		// Initialize GLEW
		glewExperimental = true; // Needed for core profile
		if (glewInit() != GLEW_OK) {
			Error("Cannot initialize GLEW");
			return false;
		}

		Info("glewInit done");

		SDL_GL_SetSwapInterval(1);

		std::string renderer = (char*)glGetString(GL_RENDERER);
		std::string version = (char*)glGetString(GL_VERSION);
		const float gl_version = float(atof(version.c_str()));
		glVersion_ = int(gl_version * 100);

		if (glVersion_ < 450)
		{
			Error("Sorry, I need at least OpenGL 4.5");
			return false;
		}

		SDL_version ver;

		SDL_GetVersion(&ver);

		Info("GL Renderer: %s", renderer.c_str());
		Info("GL Version: %s (%.2f)", version.c_str(), gl_version);
		Info("SDL version: %d.%d patch %d", (int)ver.major, (int)ver.minor, (int)ver.patch);

		int _w, _h;

		SDL_GetWindowSize(windowHandle_, &_w, &_h);
		glViewport(0, 0, _w, _h);
		glScissor(0, 0, _w, _h);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glDisable(GL_FRAMEBUFFER_SRGB);

#ifdef _DEBUG
		if (GLEW_ARB_debug_output)
		{
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

			glDebugMessageCallbackARB(&DebugMessageCallback, NULL);
			//glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
			//glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);

		}
#endif
		if (GLEW_ARB_explicit_uniform_location)
		{
			//videoConf.explicitUnifromLocationEXT = true;
		}

		SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &window_.redBits);
		SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &window_.greenBits);
		SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &window_.blueBits);
		SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &window_.depthBits);
		SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &window_.alphaBits);
		SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &window_.stencilBits);


		window_.w = _w;
		window_.h = _h;

		return true;
	}

	void OpenGLRenderContext::destroy_window()
	{
		if (glcontext_)
			SDL_GL_DeleteContext(glcontext_);
		if (windowHandle_)
			SDL_DestroyWindow(windowHandle_);
	}

	glm::ivec2 OpenGLRenderContext::get_window_size() const
	{
		return glm::ivec2(window_.w, window_.h);
	}

	OpenGLRenderContext::~OpenGLRenderContext()
	{
		destroy_window();
	}

	void OpenGLRenderContext::set_state(uint64_t stateBits, bool force)
	{
		uint64_t diff = stateBits ^ state_bits_;

		if (force)
		{
			// make sure everything is set all the time, so we
			// can see if our delta checking is screwing up
			diff = 0xFFFFFFFFFFFFFFFF;
		}
		else if (diff == 0)
		{
			return;
		}

		//
		// culling
		//
		if (diff & (GLS_CULL_MASK))//| GLS_MIRROR_VIEW ) )
		{
			switch (stateBits & GLS_CULL_MASK)
			{
			case GLS_CULL_TWOSIDED:
				glDisable(GL_CULL_FACE);
				break;

			case GLS_CULL_BACKSIDED:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				break;

			case GLS_CULL_FRONTSIDED:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				break;
			}
		}

		//
		// check depthFunc bits
		//
		if (diff & GLS_DEPTHFUNC_BITS)
		{
			switch (stateBits & GLS_DEPTHFUNC_BITS)
			{
			case GLS_DEPTHFUNC_EQUAL:
				glDepthFunc(GL_EQUAL);
				break;
			case GLS_DEPTHFUNC_ALWAYS:
				glDepthFunc(GL_ALWAYS);
				break;
			case GLS_DEPTHFUNC_LESS:
				glDepthFunc(GL_LEQUAL);
				break;
			case GLS_DEPTHFUNC_GREATER:
				glDepthFunc(GL_GEQUAL);
				break;
			}
		}

		//
		// check blend bits
		//
		if (diff & (GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS))
		{
			GLenum srcFactor = GL_ONE;
			GLenum dstFactor = GL_ZERO;

			switch (stateBits & GLS_SRCBLEND_BITS)
			{
			case GLS_SRCBLEND_ZERO:
				srcFactor = GL_ZERO;
				break;
			case GLS_SRCBLEND_ONE:
				srcFactor = GL_ONE;
				break;
			case GLS_SRCBLEND_DST_COLOR:
				srcFactor = GL_DST_COLOR;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:
				srcFactor = GL_ONE_MINUS_DST_COLOR;
				break;
			case GLS_SRCBLEND_SRC_ALPHA:
				srcFactor = GL_SRC_ALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
				srcFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case GLS_SRCBLEND_DST_ALPHA:
				srcFactor = GL_DST_ALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
				srcFactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			default:
				assert(!"GL_State: invalid src blend state bits\n");
				break;
			}

			switch (stateBits & GLS_DSTBLEND_BITS)
			{
			case GLS_DSTBLEND_ZERO:
				dstFactor = GL_ZERO;
				break;
			case GLS_DSTBLEND_ONE:
				dstFactor = GL_ONE;
				break;
			case GLS_DSTBLEND_SRC_COLOR:
				dstFactor = GL_SRC_COLOR;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:
				dstFactor = GL_ONE_MINUS_SRC_COLOR;
				break;
			case GLS_DSTBLEND_SRC_ALPHA:
				dstFactor = GL_SRC_ALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:
				dstFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case GLS_DSTBLEND_DST_ALPHA:
				dstFactor = GL_DST_ALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:
				dstFactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			default:
				assert(!"GL_State: invalid dst blend state bits\n");
				break;
			}

			// Only actually update GL's blend func if blending is enabled.
			if (srcFactor == GL_ONE && dstFactor == GL_ZERO)
			{
				glDisable(GL_BLEND);
			}
			else
			{
				glEnable(GL_BLEND);
				glBlendFunc(srcFactor, dstFactor);
			}
		}

		//
		// check depthmask
		//
		if (diff & GLS_DEPTHMASK)
		{
			if (stateBits & GLS_DEPTHMASK)
			{
				glDepthMask(GL_FALSE);
			}
			else
			{
				glDepthMask(GL_TRUE);
			}
		}

		//
		// check colormask
		//
		if (diff & (GLS_REDMASK | GLS_GREENMASK | GLS_BLUEMASK | GLS_ALPHAMASK))
		{
			GLboolean r = (stateBits & GLS_REDMASK) ? GL_FALSE : GL_TRUE;
			GLboolean g = (stateBits & GLS_GREENMASK) ? GL_FALSE : GL_TRUE;
			GLboolean b = (stateBits & GLS_BLUEMASK) ? GL_FALSE : GL_TRUE;
			GLboolean a = (stateBits & GLS_ALPHAMASK) ? GL_FALSE : GL_TRUE;
			glColorMask(r, g, b, a);
		}

		//
		// fill/line mode
		//
		if (diff & GLS_POLYMODE_LINE)
		{
			if (stateBits & GLS_POLYMODE_LINE)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}

		//
		// polygon offset
		//
		if (diff & GLS_POLYGON_OFFSET)
		{
			if (stateBits & GLS_POLYGON_OFFSET)
			{
				glPolygonOffset(polyOfsScale_, polyOfsBias_);
				glEnable(GL_POLYGON_OFFSET_FILL);
				glEnable(GL_POLYGON_OFFSET_LINE);
			}
			else
			{
				glDisable(GL_POLYGON_OFFSET_FILL);
				glDisable(GL_POLYGON_OFFSET_LINE);
			}
		}

		//
		// stencil
		//
		if (diff & (GLS_STENCIL_FUNC_BITS | GLS_STENCIL_OP_BITS))
		{
			if ((stateBits & (GLS_STENCIL_FUNC_BITS | GLS_STENCIL_OP_BITS)) != 0)
			{
				glEnable(GL_STENCIL_TEST);
			}
			else
			{
				glDisable(GL_STENCIL_TEST);
			}
		}
		if (diff & (GLS_STENCIL_FUNC_BITS | GLS_STENCIL_FUNC_REF_BITS | GLS_STENCIL_FUNC_MASK_BITS))
		{
			GLuint ref = GLuint((stateBits & GLS_STENCIL_FUNC_REF_BITS) >> GLS_STENCIL_FUNC_REF_SHIFT);
			GLuint mask = GLuint((stateBits & GLS_STENCIL_FUNC_MASK_BITS) >> GLS_STENCIL_FUNC_MASK_SHIFT);
			GLenum func = 0;

			switch (stateBits & GLS_STENCIL_FUNC_BITS)
			{
			case GLS_STENCIL_FUNC_NEVER:
				func = GL_NEVER;
				break;
			case GLS_STENCIL_FUNC_LESS:
				func = GL_LESS;
				break;
			case GLS_STENCIL_FUNC_EQUAL:
				func = GL_EQUAL;
				break;
			case GLS_STENCIL_FUNC_LEQUAL:
				func = GL_LEQUAL;
				break;
			case GLS_STENCIL_FUNC_GREATER:
				func = GL_GREATER;
				break;
			case GLS_STENCIL_FUNC_NOTEQUAL:
				func = GL_NOTEQUAL;
				break;
			case GLS_STENCIL_FUNC_GEQUAL:
				func = GL_GEQUAL;
				break;
			case GLS_STENCIL_FUNC_ALWAYS:
				func = GL_ALWAYS;
				break;
			}
			glStencilFunc(func, ref, mask);
		}
		if (diff & (GLS_STENCIL_OP_FAIL_BITS | GLS_STENCIL_OP_ZFAIL_BITS | GLS_STENCIL_OP_PASS_BITS))
		{
			GLenum sFail = 0;
			GLenum zFail = 0;
			GLenum pass = 0;

			switch (stateBits & GLS_STENCIL_OP_FAIL_BITS)
			{
			case GLS_STENCIL_OP_FAIL_KEEP:
				sFail = GL_KEEP;
				break;
			case GLS_STENCIL_OP_FAIL_ZERO:
				sFail = GL_ZERO;
				break;
			case GLS_STENCIL_OP_FAIL_REPLACE:
				sFail = GL_REPLACE;
				break;
			case GLS_STENCIL_OP_FAIL_INCR:
				sFail = GL_INCR;
				break;
			case GLS_STENCIL_OP_FAIL_DECR:
				sFail = GL_DECR;
				break;
			case GLS_STENCIL_OP_FAIL_INVERT:
				sFail = GL_INVERT;
				break;
			case GLS_STENCIL_OP_FAIL_INCR_WRAP:
				sFail = GL_INCR_WRAP;
				break;
			case GLS_STENCIL_OP_FAIL_DECR_WRAP:
				sFail = GL_DECR_WRAP;
				break;
			}
			switch (stateBits & GLS_STENCIL_OP_ZFAIL_BITS)
			{
			case GLS_STENCIL_OP_ZFAIL_KEEP:
				zFail = GL_KEEP;
				break;
			case GLS_STENCIL_OP_ZFAIL_ZERO:
				zFail = GL_ZERO;
				break;
			case GLS_STENCIL_OP_ZFAIL_REPLACE:
				zFail = GL_REPLACE;
				break;
			case GLS_STENCIL_OP_ZFAIL_INCR:
				zFail = GL_INCR;
				break;
			case GLS_STENCIL_OP_ZFAIL_DECR:
				zFail = GL_DECR;
				break;
			case GLS_STENCIL_OP_ZFAIL_INVERT:
				zFail = GL_INVERT;
				break;
			case GLS_STENCIL_OP_ZFAIL_INCR_WRAP:
				zFail = GL_INCR_WRAP;
				break;
			case GLS_STENCIL_OP_ZFAIL_DECR_WRAP:
				zFail = GL_DECR_WRAP;
				break;
			}
			switch (stateBits & GLS_STENCIL_OP_PASS_BITS)
			{
			case GLS_STENCIL_OP_PASS_KEEP:
				pass = GL_KEEP;
				break;
			case GLS_STENCIL_OP_PASS_ZERO:
				pass = GL_ZERO;
				break;
			case GLS_STENCIL_OP_PASS_REPLACE:
				pass = GL_REPLACE;
				break;
			case GLS_STENCIL_OP_PASS_INCR:
				pass = GL_INCR;
				break;
			case GLS_STENCIL_OP_PASS_DECR:
				pass = GL_DECR;
				break;
			case GLS_STENCIL_OP_PASS_INVERT:
				pass = GL_INVERT;
				break;
			case GLS_STENCIL_OP_PASS_INCR_WRAP:
				pass = GL_INCR_WRAP;
				break;
			case GLS_STENCIL_OP_PASS_DECR_WRAP:
				pass = GL_DECR_WRAP;
				break;
			}
			glStencilOp(sFail, zFail, pass);
		}

		state_bits_ = stateBits;
	}


	void GLAPIENTRY DebugMessageCallback(GLenum source,
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
}
