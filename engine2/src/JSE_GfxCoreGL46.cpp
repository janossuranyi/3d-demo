#include <GL/glew.h>
#include "JSE.h"
#include "JSE_GfxCoreGL46.h"

GLenum MapJseBufferTarget(JseBufferTarget t);

static void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		Error("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
		abort();
	}
}

void GLAPIENTRY JSE_DebugMessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const GLvoid* userParam);


JseGfxCoreGL::~JseGfxCoreGL()
{
	Shutdown_impl();
}

JseResult JseGfxCoreGL::Init_impl(bool debugMode)
{
    useDebugMode_ = debugMode;

    return JseResult::SUCCESS;
}

JseResult JseGfxCoreGL::CreateSurface_impl(const JseSurfaceCreateInfo& createSurfaceInfo)
{

	int err;

	if ((err = SDL_Init(SDL_INIT_VIDEO) < 0)) {
		Error("ERROR: %s", SDL_GetError());
		return JseResult::GENERIC_ERROR;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, createSurfaceInfo.depthBits);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, createSurfaceInfo.colorBits / 3);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, createSurfaceInfo.colorBits / 3);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, createSurfaceInfo.colorBits / 3);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, createSurfaceInfo.alphaBits);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, createSurfaceInfo.stencilBits);

	//SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, SDL_FALSE);
	if (useDebugMode_) {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	}
	//SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, SDL_TRUE);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, awindow.multisample);

	Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

	if (createSurfaceInfo.fullScreen && (createSurfaceInfo.width == 0 || createSurfaceInfo.height == 0))
	{
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	else if (createSurfaceInfo.fullScreen)
	{
		flags |= SDL_WINDOW_FULLSCREEN;
	}

	Info("SD_CreateWindow start");
	windowHandle_ = SDL_CreateWindow("JseGfxCoreGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, createSurfaceInfo.width, createSurfaceInfo.height, flags);
	if (!windowHandle_)
	{
		Error("Cannot create window - %s", SDL_GetError());
		return JseResult::GENERIC_ERROR;
	}

	Info("SD_CreateWindow done");

	glcontext_ = SDL_GL_CreateContext(windowHandle_);
	if (!glcontext_)
	{
		Error("Cannot create GL context - %s", SDL_GetError());
		return JseResult::GENERIC_ERROR;
	}

	SDL_GL_MakeCurrent(windowHandle_, glcontext_);

	Info("SDL_GL_CreateContext done");

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		Error("Cannot initialize GLEW");
		return JseResult::GENERIC_ERROR;
	}

	Info("Using GLEW %s", glewGetString(GLEW_VERSION));

	SDL_GL_SetSwapInterval(createSurfaceInfo.swapInterval);

	std::string renderer = (char*)glGetString(GL_RENDERER);
	std::string version = (char*)glGetString(GL_VERSION);

	Info("OpenGL extensions");
	GLint numExts; glGetIntegerv(GL_NUM_EXTENSIONS, &numExts);
	for (int ext_ = 0; ext_ < numExts; ++ext_)
	{
		const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, ext_);
		//Info("%s", extension);
		gl_extensions_.emplace(extension);
	}
	const float gl_version = float(atof(version.c_str()));
	glVersion_ = int(gl_version * 100);

	if (glVersion_ < 450)
	{
		Error("GL_VERSION < 4.5");
		return JseResult::GENERIC_ERROR;
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
	if (gl_extensions_.count("GL_ARB_debug_output"))
	{
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

		glDebugMessageCallbackARB(&JSE_DebugMessageCallback, NULL);

		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR_ARB, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DEBUG_TYPE_PERFORMANCE_ARB, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DEBUG_TYPE_PORTABILITY_ARB, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB, GL_DONT_CARE, 0, nullptr, GL_TRUE);

		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);

	}
#endif
	return JseResult::SUCCESS;
}

JseResult JseGfxCoreGL::CreateBuffer_impl(const JseBufferCreateInfo& cmd)
{
	auto find = buffer_data_map_.find(cmd.bufferId);

	// buffer exists
	if (find != std::end(buffer_data_map_)) return JseResult::ALREADY_EXISTS;
	BufferData bufferData{};

	GLbitfield flags{};
	{
		if (cmd.storageFlags & JSE_BUFFER_STORAGE_READ_BIT)			flags |= GL_MAP_READ_BIT;
		if (cmd.storageFlags & JSE_BUFFER_STORAGE_WRITE_BIT)		flags |= GL_MAP_WRITE_BIT;
		if (cmd.storageFlags & JSE_BUFFER_STORAGE_COHERENT_BIT)		flags |= GL_MAP_COHERENT_BIT;
		if (cmd.storageFlags & JSE_BUFFER_STORAGE_PERSISTENT_BIT)	flags |= GL_MAP_PERSISTENT_BIT;
		if (cmd.storageFlags & JSE_BUFFER_STORAGE_DYNAMIC_BIT)		flags |= GL_DYNAMIC_STORAGE_BIT;
	}

	const GLsizeiptr _size = static_cast<GLsizeiptr>(cmd.size);

	GL_CHECK(glCreateBuffers(1, &bufferData.buffer));
	GL_CHECK(glNamedBufferStorage(bufferData.buffer, _size, nullptr, flags));

	if (cmd.storageFlags & JSE_BUFFER_STORAGE_PERSISTENT_BIT) {
		GL_CHECK(bufferData.mapptr = glMapNamedBufferRange(bufferData.buffer, 0, _size, flags));
	}

	bufferData.size = _size;
	bufferData.target = MapJseBufferTarget(cmd.target);

	buffer_data_map_.emplace(cmd.bufferId, bufferData);

	return JseResult::SUCCESS;
}

JseResult JseGfxCoreGL::UpdateBuffer_impl(const JseBufferUpdateInfo& cmd)
{
	auto find = buffer_data_map_.find(cmd.bufferId);

	if (find != std::end(buffer_data_map_)) {
		if (find->second.size < (cmd.offset + cmd.data.size())) {
			return JseResult::INTERVAL_EXCEEDED;
		}
		if (find->second.mapptr) {
			std::memcpy(reinterpret_cast<char*>(find->second.mapptr) + cmd.offset, cmd.data.data(), cmd.data.size());
		} else {
			GL_CHECK(glNamedBufferSubData(find->second.buffer, cmd.offset, cmd.data.size(), cmd.data.data()));
		}
		return JseResult::SUCCESS;
	}

	return JseResult::NOT_EXISTS;
}

JseResult JseGfxCoreGL::DestroyBuffer_impl(JseBufferID bufferId)
{
	auto find = buffer_data_map_.find(bufferId);

	if (find != std::end(buffer_data_map_)) {
		if (find->second.mapptr) {
			GL_CHECK(glUnmapNamedBuffer(find->second.buffer));
		}
		GL_CHECK(glDeleteBuffers(1, &find->second.buffer));

		buffer_data_map_.erase(bufferId);

		return JseResult::SUCCESS;
	}
	
	return JseResult::NOT_EXISTS;

}

JseResult JseGfxCoreGL::CreateImage_impl(const JseImageCreateInfo& cmd)
{
	auto find = texture_data_map_.find(cmd.imageId);

	if (find == std::end(texture_data_map_)) {
		return JseResult::ALREADY_EXISTS;
	}

	

    return JseResult();
}

JseResult JseGfxCoreGL::CreateGraphicsPipeline_impl(const JseGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo)
{
    return JseResult();
}

void JseGfxCoreGL::Shutdown_impl()
{
	if (glcontext_) {
		SDL_GL_DeleteContext(glcontext_);
		glcontext_ = 0;
	}
	if (windowHandle_) {
		SDL_DestroyWindow(windowHandle_);
		windowHandle_ = nullptr;
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

	static GLenum MapJseBufferTarget(JseBufferTarget t) {
		switch (t) {
		case JseBufferTarget::VERTEX:
			return GL_ARRAY_BUFFER;
		case JseBufferTarget::INDEX:
			return GL_ELEMENT_ARRAY_BUFFER;
		case JseBufferTarget::TEXTURE:
			return GL_TEXTURE_BUFFER;
		case JseBufferTarget::UNIFORM:
		case JseBufferTarget::UNIFORM_DYNAMIC:
			return GL_UNIFORM_BUFFER;
		case JseBufferTarget::SHADER_STORAGE:
		case JseBufferTarget::SHADER_STORAGE_DYNAMIC:
			return GL_SHADER_STORAGE_BUFFER;
		}
	}