#include <GL/glew.h>
#include "JSE.h"
#include "JSE_GfxCoreGL46.h"

GLenum MapJseBufferTargetGl(JseBufferTarget t);
GLenum MapJseFilterGl(JseFilter t);
GLenum MapJseTilingGl(JseImageTiling t);
GLenum MapJseImageTargetGl(JseImageTarget t);
GLenum MapJseCubemapFaceGl(JseCubeMapFace t);
GLenum MapJseShaderStageGl(JseShaderStage t);
GLenum MapJseAccessImageAccessGl(JseAccess t);
GLenum MapJseTopologyGl(JseTopology p);
GLenum MapTexureTargetToBinding(GLenum target);

JseFormat MapGlFormatToJse(GLenum format);

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

class GLUniformBinder {
public:
	GLUniformBinder() : uniform_location_{ 0 } {
	}

	~GLUniformBinder() = default;

	void operator()(const int& value) {
		GL_CHECK(glUniform1i(uniform_location_, value));
	}
	void operator()(const float& value) {
		GL_CHECK(glUniform1f(uniform_location_, value));
	}
	void operator()(const glm::ivec2& value) {
		GL_CHECK(glUniform2iv(uniform_location_, 1, &value[0]));
	}
	void operator()(const glm::ivec3& value) {
		GL_CHECK(glUniform3iv(uniform_location_, 1, &value[0]));
	}
	void operator()(const glm::ivec4& value) {
		GL_CHECK(glUniform4iv(uniform_location_, 1, &value[0]));
	}
	void operator()(const glm::vec2& value) {
		GL_CHECK(glUniform2fv(uniform_location_, 1, &value[0]));
	}
	void operator()(const glm::vec3& value) {
		GL_CHECK(glUniform3fv(uniform_location_, 1, &value[0]));
	}
	void operator()(const glm::vec4& value) {
		GL_CHECK(glUniform4fv(uniform_location_, 1, &value[0]));
	}
	void operator()(const glm::mat3& value) {
		GL_CHECK(glUniformMatrix3fv(uniform_location_, 1, GL_FALSE, &value[0][0]));
	}
	void operator()(const glm::mat4& value) {
		GL_CHECK(glUniformMatrix4fv(uniform_location_, 1, GL_FALSE, &value[0][0]));
	}
	void operator()(const JsVector<float>& value) {
		GL_CHECK(glUniform1fv(uniform_location_, value.size(), (const GLfloat*)value.data()));
	}
	void operator()(const JsVector<glm::vec4>& value) {
		GL_CHECK(glUniform4fv(uniform_location_, value.size(), (const GLfloat*)value.data()));
	}
	void update(GLint location, const JseUniformData& value) {
		uniform_location_ = location;
		std::visit(*this, value);
	}
private:
	GLint uniform_location_;
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

namespace js {

GfxCoreGL::~GfxCoreGL()
{
	Shutdown_impl();
	Info("OpenGL core deinitialized.");
}

GfxCoreGL::GfxCoreGL() : 
	windowHandle_(nullptr),
	useDebugMode_(false),
	glcontext_(0),
	glVersion_(0)
{
	Info("GfxCore OpenGL 4.6 Driver v1.0");
}

void* GfxCoreGL::GetMappedBufferPointer_impl(JseBufferID id)
{
	auto find = buffer_data_map_.find(id);

	return find != std::end(buffer_data_map_) ? find->second.mapptr : nullptr;
}

Result GfxCoreGL::Init_impl(bool debugMode)
{
    useDebugMode_ = debugMode;

    return Result::SUCCESS;
}

Result GfxCoreGL::CreateSurface_impl(const JseSurfaceCreateInfo& createSurfaceInfo)
{

	int err;

	if ((err = SDL_Init(SDL_INIT_VIDEO) < 0)) {
		Error("ERROR: %s", SDL_GetError());
		return Result::GENERIC_ERROR;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, createSurfaceInfo.depthBits);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, createSurfaceInfo.colorBits / 3);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, createSurfaceInfo.colorBits / 3);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, createSurfaceInfo.colorBits / 3);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, createSurfaceInfo.alphaBits);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, createSurfaceInfo.stencilBits);

	if (createSurfaceInfo.srgb) {
		SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, SDL_TRUE);
	}
#if _DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
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

	Info("SDL_CreateWindow start");
	windowHandle_ = SDL_CreateWindow("GfxCoreGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, createSurfaceInfo.width, createSurfaceInfo.height, flags);
	if (!windowHandle_)
	{
		Error("Cannot create window - %s", SDL_GetError());
		return Result::GENERIC_ERROR;
	}

	Info("SDL_CreateWindow done");

	glcontext_ = SDL_GL_CreateContext(windowHandle_);
	if (!glcontext_)
	{
		Error("Cannot create GL context - %s", SDL_GetError());
		return Result::GENERIC_ERROR;
	}

	//SDL_GL_MakeCurrent(windowHandle_, glcontext_);

	Info("SDL_GL_CreateContext done");

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		Error("Cannot initialize GLEW");
		return Result::GENERIC_ERROR;
	}

	Info("Using GLEW %s", glewGetString(GLEW_VERSION));

	SDL_GL_SetSwapInterval(createSurfaceInfo.swapInterval);

	std::string renderer = (char*)glGetString(GL_RENDERER);
	std::string version = (char*)glGetString(GL_VERSION);

	Info("OpenGL extensions");
	GLint numExts; glGetIntegerv(GL_NUM_EXTENSIONS, &numExts);
	JsString exts;

	for (int ext_ = 0; ext_ < numExts; ++ext_)
	{
		const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, ext_);
		//Info("%s", extension);
		gl_extensions_.emplace(extension);
		exts.append(extension).append(" ");
	}
	const float gl_version = float(atof(version.c_str()));
	glVersion_ = int(gl_version * 100);

	if (glVersion_ < 450)
	{
		Error("GL_VERSION < 4.5");
		return Result::GENERIC_ERROR;
	}

	Info("extensions: %s", exts.c_str());
	SDL_version ver;

	SDL_GetVersion(&ver);

	Info("GL Renderer: %s", renderer.c_str());
	Info("GL Version: %s (%.2f)", version.c_str(), gl_version);
	Info("SDL version: %d.%d patch %d", (int)ver.major, (int)ver.minor, (int)ver.patch);

	int _w, _h;

	SDL_GL_GetDrawableSize(windowHandle_, &_w, &_h);
	glViewport(0, 0, _w, _h);
	glScissor(0, 0, _w, _h);
	stateCache_.viewport = JseRect2D{ 0,0,_w,_h };
	stateCache_.scissor  = JseRect2D{ 0,0,_w,_h };

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	if (createSurfaceInfo.srgb) {
		glEnable(GL_FRAMEBUFFER_SRGB);
	}

	deviceCapabilities_.pRenderer			= (const char*)glGetString(GL_RENDERER);
	deviceCapabilities_.pRendererVersion	= (const char*)glGetString(GL_VERSION);
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &deviceCapabilities_.maxArrayTextureLayers);
	
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &deviceCapabilities_.maxFragmentTextureImageUnits);
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &deviceCapabilities_.maxVertexTextureImageUnits);
	glGetIntegerv(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, &deviceCapabilities_.maxComputeTextureImageUnits);

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &deviceCapabilities_.maxTextureSize);
	glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &deviceCapabilities_.maxComputeSharedMemorySize);
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &deviceCapabilities_.maxUniformBlockSize);
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &deviceCapabilities_.maxShaderStorageBlockSize);
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &deviceCapabilities_.maxVertexAttribs);
	glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &deviceCapabilities_.maxVertexAttribBindings);

	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &deviceCapabilities_.uniformBufferOffsetAligment);
	glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &deviceCapabilities_.availableVideoMemory);

	glGetError();
	glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, &deviceCapabilities_.availableVideoMemory);
	glGetError();
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_DEPTH_TEST);

#ifdef _DEBUG
	if (gl_extensions_.count("GL_ARB_debug_output"))
	{
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

		glDebugMessageCallbackARB(&JSE_DebugMessageCallback, NULL);
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
	return Result::SUCCESS;
}

Result GfxCoreGL::CreateBuffer_impl(const JseBufferCreateInfo& cmd)
{
	auto find = buffer_data_map_.find(cmd.bufferId);

	// buffer exists
	if (find != std::end(buffer_data_map_)) return Result::ALREADY_EXISTS;
	BufferData bufferData{};

	GLbitfield flags{};
	{
		if (cmd.storageFlags & JSE_BUFFER_STORAGE_READ_BIT)			flags |= GL_MAP_READ_BIT;
		if (cmd.storageFlags & JSE_BUFFER_STORAGE_WRITE_BIT)		flags |= GL_MAP_WRITE_BIT;
		if (cmd.storageFlags & JSE_BUFFER_STORAGE_COHERENT_BIT)		flags |= GL_MAP_COHERENT_BIT;
		if (cmd.storageFlags & JSE_BUFFER_STORAGE_PERSISTENT_BIT)	flags |= GL_MAP_PERSISTENT_BIT;
		if (cmd.storageFlags & JSE_BUFFER_STORAGE_DYNAMIC_BIT)		flags |= GL_DYNAMIC_STORAGE_BIT;
	}

	if (!flags) {
		flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT;
	}

	const GLsizeiptr _size = static_cast<GLsizeiptr>(cmd.size);

	GL_CHECK(glCreateBuffers(1, &bufferData.buffer));
	GL_CHECK(glNamedBufferStorage(bufferData.buffer, _size, nullptr, flags));

	if (cmd.storageFlags & JSE_BUFFER_STORAGE_PERSISTENT_BIT) {
		flags = flags & ~GL_DYNAMIC_STORAGE_BIT;
		GL_CHECK(bufferData.mapptr = glMapNamedBufferRange(bufferData.buffer, 0, _size, flags));
	}

	bufferData.size = _size;
	bufferData.target = MapJseBufferTargetGl(cmd.target);

	buffer_data_map_.emplace(cmd.bufferId, bufferData);

	return Result::SUCCESS;
}

Result GfxCoreGL::UpdateBuffer_impl(const JseBufferUpdateInfo& cmd)
{
	auto find = buffer_data_map_.find(cmd.bufferId);

	if (find != std::end(buffer_data_map_)) {
		if (find->second.size < (cmd.offset + cmd.size)) {
			return Result::INTERVAL_EXCEEDED;
		}
		if (find->second.mapptr) {
			std::memcpy(reinterpret_cast<char*>(find->second.mapptr) + cmd.offset, cmd.data, cmd.size);
		} else {
			GL_CHECK(glNamedBufferSubData(find->second.buffer, cmd.offset, cmd.size, cmd.data));
		}
		return Result::SUCCESS;
	}

	return Result::NOT_EXISTS;
}

Result GfxCoreGL::DestroyBuffer_impl(JseBufferID bufferId)
{
	auto find = buffer_data_map_.find(bufferId);

	if (find != std::end(buffer_data_map_)) {
		if (find->second.mapptr) {
			GL_CHECK(glUnmapNamedBuffer(find->second.buffer));
		}
		GL_CHECK(glDeleteBuffers(1, &find->second.buffer));

		buffer_data_map_.erase(bufferId);

		return Result::SUCCESS;
	}
	
	return Result::NOT_EXISTS;

}

Result GfxCoreGL::CreateImage_impl(const JseImageCreateInfo& cmd)
{
	auto find = texture_data_map_.find(cmd.imageId);

	if (find != std::end(texture_data_map_)) {
		return Result::ALREADY_EXISTS;
	}
	
	if (cmd.target == JseImageTarget::BUFFER && !cmd.buffer.isValid()) {
		return Result::INVALID_VALUE;
	}

	ImageData data{};
	TextureFormatInfo formatInfo = s_texture_format[SCAST(size_t, cmd.format)];

	data.target = MapJseImageTargetGl(cmd.target);
	if (data.target == 0) {
		return Result::INVALID_VALUE;
	}

	GLenum const internalFormat = cmd.srgb ? formatInfo.internal_format_srgb : formatInfo.internal_format;
	
	data.format = formatInfo.format;
	data.internal_format = internalFormat;
	data.type = formatInfo.type;
	GLenum binding = MapTexureTargetToBinding(data.target);
	GLint bound{};
	
	GL_CHECK(glGetIntegerv(binding, &bound));
	GL_CHECK(glGenTextures(1, &data.texture));
	GL_CHECK(glBindTexture(data.target, data.texture));

	if (cmd.target == JseImageTarget::BUFFER) {
		auto& buf = buffer_data_map_.at(cmd.buffer);
		if (cmd.offset == 0 && cmd.size == 0) {
			GL_CHECK(glTexBuffer(data.target, data.format, buf.buffer));
		}
		else {
			GL_CHECK(glTexBufferRange(data.target, data.format, buf.buffer, cmd.offset, cmd.size));
		}
	}

	if (cmd.target != JseImageTarget::BUFFER) {

		JseFilter filter = cmd.levelCount > 1 ? JseFilter::LINEAR_MIPMAP_LINEAR : JseFilter::LINEAR;

		GLenum tilingS = MapJseTilingGl(JseImageTiling::CLAMP_TO_EDGE);
		GLenum tilingT = MapJseTilingGl(JseImageTiling::CLAMP_TO_EDGE);
		GLenum tilingR = MapJseTilingGl(JseImageTiling::CLAMP_TO_EDGE);
		GLenum minFilter = MapJseFilterGl(filter);
		GLenum magFilter = MapJseFilterGl(JseFilter::LINEAR);
		GLfloat maxAnisotropy = 1.0f;
		GLfloat minLod = 0.0f;
		GLfloat maxLod = SCAST(float, cmd.levelCount);
		GLfloat lodBias = 0.0f;
		Color4f borderColor{ 0.0f,0.0f,0.0f,1.0f };

		if (cmd.samplerDescription) {

			tilingS = MapJseTilingGl(cmd.samplerDescription->tilingS);
			tilingT = MapJseTilingGl(cmd.samplerDescription->tilingT);
			tilingR = MapJseTilingGl(cmd.samplerDescription->tilingR);
			minFilter = MapJseFilterGl(cmd.samplerDescription->minFilter);
			magFilter = MapJseFilterGl(cmd.samplerDescription->magFilter);
			minLod = GLfloat(cmd.samplerDescription->minLod);
			maxLod = GLfloat(cmd.samplerDescription->maxLod);
			maxAnisotropy = GLfloat(cmd.samplerDescription->maxAnisotropy);
			lodBias = GLfloat(cmd.samplerDescription->lodBias);
			borderColor = cmd.samplerDescription->borderColor;
		}

		// glTextureParameterf ...
		GL_CHECK(glTexParameteri(data.target, GL_TEXTURE_MIN_FILTER, minFilter));
		GL_CHECK(glTexParameteri(data.target, GL_TEXTURE_MAG_FILTER, magFilter));
		GL_CHECK(glTexParameteri(data.target, GL_TEXTURE_WRAP_S, tilingS));
		GL_CHECK(glTexParameteri(data.target, GL_TEXTURE_WRAP_T, tilingT));
		GL_CHECK(glTexParameteri(data.target, GL_TEXTURE_WRAP_R, tilingR));
		GL_CHECK(glTexParameterf(data.target, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy));
		GL_CHECK(glTexParameterf(data.target, GL_TEXTURE_MIN_LOD, minLod));
		GL_CHECK(glTexParameterf(data.target, GL_TEXTURE_MAX_LOD, maxLod));
		GL_CHECK(glTexParameterf(data.target, GL_TEXTURE_LOD_BIAS, lodBias));
		GL_CHECK(glTexParameterfv(data.target, GL_TEXTURE_BORDER_COLOR, &borderColor.r));
	}

	if (cmd.immutable) {
		data.immutable = true;
		switch(data.target) {
		case GL_TEXTURE_1D:
			GL_CHECK(glTexStorage1D(data.target, cmd.levelCount, data.internal_format, cmd.width));
			break;
		case GL_TEXTURE_1D_ARRAY:
		case GL_TEXTURE_2D:
		case GL_TEXTURE_CUBE_MAP:
			GL_CHECK(glTexStorage2D(data.target, cmd.levelCount, data.internal_format, cmd.width, cmd.height));
			break;
		case GL_TEXTURE_2D_ARRAY:
		case GL_TEXTURE_CUBE_MAP_ARRAY:
		case GL_TEXTURE_3D:
			GL_CHECK(glTexStorage3D(data.target, cmd.levelCount, data.internal_format, cmd.width, cmd.height, cmd.depth));
			break;
		}

		if (cmd.compressed) {
			/*
			* when using CompressedTex*Subimage* functions, the format must be the exact internal format
			*/
			GLint format{SCAST(GLint, data.internal_format)};
			GLint compressed{-1};
			GLenum const target = data.target != GL_TEXTURE_CUBE_MAP ? data.target : GL_TEXTURE_CUBE_MAP_POSITIVE_X;
			glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, &format); glGetError();

			glGetTexLevelParameteriv(target, 0, GL_TEXTURE_COMPRESSED, &compressed); glGetError();
			Info("%d isCompressed: %d", cmd.imageId, compressed);

			data.format = format;
		}
	}

	GL_CHECK(glBindTexture(data.target, bound));

	data.compressed = cmd.compressed;
	texture_data_map_.emplace(cmd.imageId, data);

	return Result::SUCCESS;
}

Result GfxCoreGL::DeleteImage_impl(JseImageID imageId)
{
	auto find = texture_data_map_.find(imageId);

	if (find == std::end(texture_data_map_)) return Result::NOT_EXISTS;

	GL_CHECK(glDeleteTextures(1, &find->second.texture));

	texture_data_map_.erase(imageId);

	return Result::SUCCESS;
}

Result GfxCoreGL::CreateGraphicsPipeline_impl(const JseGraphicsPipelineCreateInfo& cmd)
{

	if (pipeline_data_map_.count(cmd.graphicsPipelineId) > 0) {
		return Result::ALREADY_EXISTS;
	}

	Result res{ Result::SUCCESS };

	GfxPipelineData data{};
	GL_CHECK(glCreateVertexArrays(1, &data.vao));
	GL_CHECK(glBindVertexArray(data.vao));
	auto* attrDesc = cmd.pVertexInputState->pAttributes;
	auto* bindDesc = cmd.pVertexInputState->pBindings;


	for (int i = 0; i < cmd.pVertexInputState->attributeCount; ++i) {
		auto& attr = attrDesc[i];
		auto& fmt = s_texture_format[SCAST(size_t, attr.format)];
		GL_CHECK(glEnableVertexAttribArray(attr.location));
		GL_CHECK(glVertexAttribFormat(attr.location, fmt.componentCount, fmt.type, fmt.normalized, attr.offset));
		GL_CHECK(glVertexAttribBinding(attr.location, bindDesc[attr.bindig].binding));
	}

	for (int i = 0; i < cmd.pVertexInputState->bindingCount; ++i) {
		data.binding.emplace_back(bindDesc[i]);
		GL_CHECK(glVertexBindingDivisor(bindDesc[i].binding, bindDesc[i].inputRate == JseVertexInputRate::VERTEX ? 0 : 1));
	}

	SetRenderState(cmd.renderState);

	GL_CHECK(data.program = glCreateProgram());
	for (int i = 0; i < cmd.stageCount; ++i) {
		auto& shaderData = shader_map_.at(cmd.pStages[i].shader);
		GL_CHECK(glCompileShader(shaderData.shader));
		GL_CHECK(glAttachShader(data.program, shaderData.shader));
	}

	GL_CHECK(glLinkProgram(data.program));
	GLint result = GL_FALSE;

	GL_CHECK(glGetProgramiv(data.program, GL_LINK_STATUS, &result));

	if (result == GL_FALSE)
	{
		GLint infologLen;
		GL_CHECK(glGetProgramiv(data.program, GL_INFO_LOG_LENGTH, &infologLen));
		if (infologLen > 0) {
			std::vector<char> logBuf(infologLen);
			GL_CHECK(glGetProgramInfoLog(data.program, infologLen, nullptr, logBuf.data()));
			Error("Linking of shader program failed: %s", logBuf.data());
			res = Result::GENERIC_ERROR;
		}
	}

	for (int i = 0; i < cmd.stageCount; ++i) {
		auto& shaderData = shader_map_.at(cmd.pStages[i].shader);
		GL_CHECK(glDetachShader(data.program, shaderData.shader));
	}


	if (res != Result::SUCCESS) {
		glDeleteProgram(data.program);
		glDeleteVertexArrays(1, &data.vao);
	}
	else {
		//glUseProgram(data.program);
		//glUseProgram(0);
		glBindVertexArray(0);
		/*
		if (cmd.setLayoutId.isValid()) {
			auto& set = set_layout_map_.at(cmd.setLayoutId);
			data.setLayout = &set;
		}
		*/
		pipeline_data_map_.emplace(cmd.graphicsPipelineId, data);
	}

	return res;
}

Result GfxCoreGL::BindGraphicsPipeline_impl(JseGrapicsPipelineID pipelineId)
{
	if (active_pipeline_ == pipelineId) 
		return Result::SUCCESS;

	auto find = pipeline_data_map_.find(pipelineId);
	if (find == std::end(pipeline_data_map_)) {
		return Result::NOT_EXISTS;
	}

	active_pipeline_ = pipelineId;
	const auto& data = find->second;
	activePipelineData_.pData = &find->second;
	SetRenderState(data.renderState);
	GL_CHECK(glBindVertexArray(data.vao));
	GL_CHECK(glUseProgram(data.program));
	stateCache_.program = data.program;

	return Result::SUCCESS;
}

Result GfxCoreGL::DeleteGraphicsPipeline_impl(JseGrapicsPipelineID pipelineId)
{
	auto find = pipeline_data_map_.find(pipelineId);
	if (find == std::end(pipeline_data_map_)) {
		return Result::NOT_EXISTS;
	}

	auto& data = find->second;
	GL_CHECK(glDeleteProgram(data.program));
	GL_CHECK(glDeleteVertexArrays(1, &data.vao));

	pipeline_data_map_.erase(pipelineId);

	return Result::SUCCESS;
}

Result GfxCoreGL::CreateShader_impl(const JseShaderCreateInfo& cmd, std::string& errorOutput)
{
	if (shader_map_.count(cmd.shaderId) > 0)
		return Result::NOT_EXISTS;


	GLuint shader = 0xffff;
	GL_CHECK(shader = glCreateShader(MapJseShaderStageGl(cmd.stage)));

	if (shader == 0xffff) {
		return Result::GENERIC_ERROR;
	}

	const GLchar* tmp = static_cast<const GLchar*>(cmd.pCode);
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
			errorOutput = std::string(logBuf.data());
		}

		GL_CHECK(glDeleteShader(shader));
		return Result::GENERIC_ERROR;
	}

	shader_map_.emplace(cmd.shaderId, ShaderData{ shader, cmd.stage });

	return Result::SUCCESS;
}

Result GfxCoreGL::CreateFrameBuffer_impl(const JseFrameBufferCreateInfo& cmd)
{
	auto find = framebuffer_map_.find(cmd.frameBufferId);

	if (find != std::end(framebuffer_map_)) {
		return Result::ALREADY_EXISTS;
	}

	GLint bound{};
	GL_CHECK(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &bound));

	FrameBufferData data{};
	GL_CHECK(glGenFramebuffers(1, &data.framebuffer));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, data.framebuffer));
	std::vector<GLenum> draw_buffers;
	for (int i = 0; i < cmd.colorAttachmentCount; ++i) {
		const auto& img = texture_data_map_.at(cmd.pColorAttachments[i].image);
		draw_buffers.emplace_back(GL_COLOR_ATTACHMENT0 + i);

		if (img.target == GL_TEXTURE_CUBE_MAP) {
			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				draw_buffers.back(),
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<uint32_t>(cmd.pColorAttachments[i].face),
				img.texture,
				cmd.pColorAttachments[i].level);
		} else if (img.target == GL_TEXTURE_3D) {
			glFramebufferTexture3D(
				GL_FRAMEBUFFER,
				draw_buffers.back(),
				img.target,
				img.texture,
				cmd.pColorAttachments[i].level,
				cmd.pColorAttachments[i].layer);

		} else {
			GL_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, draw_buffers.back(), img.texture, cmd.pColorAttachments[i].level));
		}
	}

	if (draw_buffers.empty()) {
		GL_CHECK(glDrawBuffer(GL_NONE));
		GL_CHECK(glReadBuffer(GL_NONE));
	}
	else {
		GL_CHECK(glDrawBuffers(static_cast<GLsizei>(draw_buffers.size()), draw_buffers.data()));
	}

	if (cmd.pDepthAttachment && cmd.pDepthAttachment->image.isValid()) {
		auto& img = texture_data_map_.at(cmd.pDepthAttachment->image);

		if (img.target == GL_TEXTURE_CUBE_MAP) {
			GL_CHECK(glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_DEPTH_ATTACHMENT,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<uint32_t>(cmd.pDepthAttachment->face),
				img.texture,
				cmd.pDepthAttachment->level));
		}
		else {
			GL_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, img.texture, cmd.pDepthAttachment->level));
		}
	}
	if (cmd.pStencilAttachment && cmd.pStencilAttachment->image.isValid()) {
		auto& img = texture_data_map_.at(cmd.pStencilAttachment->image);
		GL_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, img.texture, cmd.pStencilAttachment->level));
	}

	GLenum status{};	
	GL_CHECK(status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, bound));
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		GL_CHECK(glDeleteFramebuffers(1, &data.framebuffer));
		return Result::FRAMEBUFFER_INCOMPLETE;
	}

	framebuffer_map_.emplace(cmd.frameBufferId, data);

	return Result::SUCCESS;
}

Result GfxCoreGL::DeleteFrameBuffer_impl(JseFrameBufferID framebufferId)
{
	auto find = framebuffer_map_.find(framebufferId);

	if (find == std::end(framebuffer_map_)) {
		return Result::NOT_EXISTS;
	}

	GL_CHECK(glDeleteFramebuffers(1, &find->second.framebuffer));
	framebuffer_map_.erase(framebufferId);

	return Result::SUCCESS;
}

Result GfxCoreGL::BeginRenderPass_impl(const JseRenderPassInfo& renderPassInfo)
{

	GLuint fb{ 0 };
	if (renderPassInfo.framebuffer.isValid() && renderPassInfo.framebuffer.internal() > 0) {
		auto& fbuff = framebuffer_map_.at(renderPassInfo.framebuffer);
		fb = fbuff.framebuffer;
	}

	_glBindFramebuffer(GL_FRAMEBUFFER, fb);
	_glViewport(renderPassInfo.viewport.x, renderPassInfo.viewport.y, renderPassInfo.viewport.w, renderPassInfo.viewport.h);
	if (renderPassInfo.scissorEnable) {
		_glScissorEnabled(renderPassInfo.scissorEnable);
		GL_CHECK(glScissor(renderPassInfo.scissor.x, renderPassInfo.scissor.y, renderPassInfo.scissor.w, renderPassInfo.scissor.h));
	}
	GLbitfield clearBits{};
	if (renderPassInfo.colorClearEnable) {
		clearBits |= GL_COLOR_BUFFER_BIT;
		if (stateCache_.clearColor != renderPassInfo.colorClearValue.color) {
			GL_CHECK(glClearColor(renderPassInfo.colorClearValue.color.r, renderPassInfo.colorClearValue.color.g, renderPassInfo.colorClearValue.color.b, renderPassInfo.colorClearValue.color.a));
			stateCache_.clearColor = renderPassInfo.colorClearValue.color;
		}
	}
	if (renderPassInfo.depthClearEnable) {
		clearBits |= GL_DEPTH_BUFFER_BIT;
		if (stateCache_.clearDepth != renderPassInfo.depthClearValue.depth) {
			GL_CHECK(glClearDepth(renderPassInfo.depthClearValue.depth));
			stateCache_.clearDepth = renderPassInfo.depthClearValue.depth;
		}
	}
	if (renderPassInfo.stencilClearEnable) {
		clearBits |= GL_STENCIL_BUFFER_BIT;
		if (stateCache_.clearStencil != renderPassInfo.stencilClearValue.stencil) {
			GL_CHECK(glClearStencil(renderPassInfo.stencilClearValue.stencil));
			stateCache_.clearStencil = renderPassInfo.stencilClearValue.stencil;
		}
	}

	if (clearBits) {
		GL_CHECK(glClear(clearBits));
	}

	return Result::SUCCESS;
}

Result GfxCoreGL::CreateDescriptorSetLayout_impl(const JseDescriptorSetLayoutCreateInfo& cmd)
{
	auto find = set_layout_map_.find(cmd.setLayoutId);
	if (find != std::end(set_layout_map_)) {
		return Result::ALREADY_EXISTS;
	}

	SetLayoutData data{};
	for (int i = 0; i < cmd.bindingCount; ++i) {
		auto& _b = cmd.pBindings[i];
		data.bindings.emplace(_b.binding, _b);
	}

	set_layout_map_.emplace(cmd.setLayoutId, data);

	return Result::SUCCESS;
}

Result GfxCoreGL::EndRenderPass_impl()
{
	_glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return Result::SUCCESS;
}

Result GfxCoreGL::CreateDescriptorSet_impl(const JseDescriptorSetCreateInfo& cmd)
{
	auto find = set_data_map_.find(cmd.setId);

	if (find != std::end(set_data_map_)) {
		return Result::ALREADY_EXISTS;
	}

	const auto& layout = set_layout_map_.at(cmd.setLayoutId);

	DescriptorSetData data{};
	data.setLayout = cmd.setLayoutId;
	data.pLayoutData = &layout;
	
	for (const auto& elem : layout.bindings) {
		switch (elem.second.descriptorType) {
		case JseDescriptorType::UNIFORM_BUFFER:
		case JseDescriptorType::UNIFORM_BUFFER_DYNAMIC:
		case JseDescriptorType::STORAGE_BUFFER:
		case JseDescriptorType::STORAGE_BUFFER_DYNAMIC:
		{
			DescriptorBufferData bd{};
			bd.binding = elem.second.binding;
			bd.type = elem.second.descriptorType;
			data.buffers.emplace_back(bd);
		}
			break;
		case JseDescriptorType::STORAGE_IMAGE:
		case JseDescriptorType::SAMPLED_IMAGE:
		case JseDescriptorType::SAMPLED_BUFFER:
		{
			DescriptorImageData id{};
			id.binding = elem.second.binding;
			id.type = elem.second.descriptorType;
			data.images.emplace_back(id);
		}
			break;
		case JseDescriptorType::INLINE_UNIFORM_BLOCK:
			;
		break;
		}
	}
	set_data_map_.emplace(cmd.setId, data);

	return Result::SUCCESS;
}

Result GfxCoreGL::BindDescriptorSet_impl(uint32_t firstSet, uint32_t descriptorSetCount, const JseDescriptorSetID* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
{
	for (int i = 0; i < descriptorSetCount; ++i) {
		auto find = set_data_map_.find(pDescriptorSets[i]);

		if (find == std::end(set_data_map_)) {
			continue;
		}

		auto& data = find->second;
		int dynIndex = 0;
		for (const auto& elem : data.buffers) {
			switch (elem.type) {
			case JseDescriptorType::UNIFORM_BUFFER:
				if (elem.size == 0ULL) {
					GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, elem.binding, elem.buffer));
				}
				else {
					GL_CHECK(glBindBufferRange(GL_UNIFORM_BUFFER, elem.binding, elem.buffer, elem.offset, elem.size));
				}
				break;

			case JseDescriptorType::UNIFORM_BUFFER_DYNAMIC:
				if (dynamicOffsetCount > 0 && dynamicOffsetCount > dynIndex) {
					GL_CHECK(glBindBufferRange(GL_UNIFORM_BUFFER, elem.binding, elem.buffer, elem.offset + static_cast<GLintptr>(pDynamicOffsets[dynIndex++]), elem.size));
				}
				break;

			case JseDescriptorType::STORAGE_BUFFER:
				if (elem.size == 0ULL) {
					GL_CHECK(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, elem.binding, elem.buffer));
				}
				else {
					GL_CHECK(glBindBufferRange(GL_SHADER_STORAGE_BUFFER, elem.binding, elem.buffer, elem.offset, elem.size));
				}
				break;

				case JseDescriptorType::STORAGE_BUFFER_DYNAMIC:
				if (dynamicOffsetCount > 0 && dynamicOffsetCount > dynIndex) {
					GL_CHECK(glBindBufferRange(GL_SHADER_STORAGE_BUFFER, elem.binding, elem.buffer, elem.offset + static_cast<GLintptr>(pDynamicOffsets[dynIndex++]), elem.size));
					++dynIndex;
				}
			}
		}

		for (const auto& elem : data.images) {
			switch (elem.type) {
			case JseDescriptorType::SAMPLED_IMAGE:
			case JseDescriptorType::SAMPLED_BUFFER:
				GL_CHECK(glBindMultiTextureEXT(GL_TEXTURE0 + elem.binding, elem.target, elem.texture));
				break;
			case JseDescriptorType::STORAGE_IMAGE:
				GL_CHECK(glBindImageTexture(elem.binding, elem.texture, elem.level, elem.layered, elem.layer, elem.access, elem.format));
			}
		}

		if (!data.uniforms.empty()) {
			SetUniforms(data, data.uniforms);
		}
	}

	return Result::SUCCESS;
}

Result GfxCoreGL::WriteDescriptorSet_impl(const JseWriteDescriptorSet& cmd)
{
	auto find = set_data_map_.find(cmd.setId);
	if (find == std::end(set_data_map_)) {
		return Result::NOT_EXISTS;
	}

	auto& data = find->second;
	auto binding = data.pLayoutData->bindings.find(cmd.dstBinding);
	if (binding == std::end(data.pLayoutData->bindings)) {
		return Result::INVALID_VALUE;
	}

	assert(cmd.descriptorType == binding->second.descriptorType);

	for (int i = 0; i < cmd.descriptorCount; ++i) {

		switch (cmd.descriptorType) {
		case JseDescriptorType::UNIFORM_BUFFER:
		case JseDescriptorType::UNIFORM_BUFFER_DYNAMIC:
		case JseDescriptorType::STORAGE_BUFFER:
		case JseDescriptorType::STORAGE_BUFFER_DYNAMIC:
			for (auto& elem : data.buffers) {
				if (elem.binding == cmd.dstBinding) {
					const auto& bdata = buffer_data_map_.at(cmd.pBufferInfo[i].buffer);
					elem.buffer = bdata.buffer;
					elem.offset = cmd.pBufferInfo[i].offset;
					elem.size = cmd.pBufferInfo[i].size;
					break;
				}
			}
			break;
		case JseDescriptorType::SAMPLED_IMAGE:
		case JseDescriptorType::SAMPLED_BUFFER:
			for (auto& elem : data.images) {
				if (elem.binding == cmd.dstBinding) {
					const auto& idata = texture_data_map_.at(cmd.pImageInfo[i].image);
					elem.texture = idata.texture;
					elem.type = cmd.descriptorType;
					elem.target = idata.target;
					break;
				}
			}
			break;
		case JseDescriptorType::STORAGE_IMAGE:
			for (auto& elem : data.images) {
				if (elem.binding == cmd.dstBinding) {
					const auto& idata = texture_data_map_.at(cmd.pImageInfo[i].image);
					elem.texture = idata.texture;
					elem.type = cmd.descriptorType;
					elem.access = MapJseAccessImageAccessGl(cmd.pImageInfo[i].access);
					elem.format = s_texture_format[static_cast<size_t>(cmd.pImageInfo[i].format)].internal_format;
					elem.layer = cmd.pImageInfo[i].layer;
					elem.level = cmd.pImageInfo[i].level;
					elem.layered = cmd.pImageInfo[i].layered;
					break;
				}
			}
			break;
		case JseDescriptorType::INLINE_UNIFORM_BLOCK:
		{
			data.uniforms.clear();
			JsVector<glm::vec4> tmp;
			for (int i = 0; i < cmd.descriptorCount; ++i) {
				auto& uniforms = cmd.pUniformInfo[i];
				if (uniforms.vectorCount) {
					tmp.assign(uniforms.pVectors, uniforms.pVectors + uniforms.vectorCount);
					data.uniforms[uniforms.name] = tmp;
				}
				else {
					data.uniforms.emplace(uniforms.name, uniforms.value);
				}
			}
		}
			break;
		default:
			Error("Unhadled DescriptorType");
		}

		
	}
	return Result::SUCCESS;
}

Result GfxCoreGL::CreateFence_impl(JseFenceID id)
{
	auto it = fence_map_.find(id);

	if (it != fence_map_.end()) {
#ifdef _DEBUG
		Info("Fence %d already exists", id.internal());
#endif // DEBUG

		return Result::ALREADY_EXISTS;
	}

	GLsync sync{};
	GL_CHECK(sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));

	if (sync) {
		fence_map_.emplace(id, sync);
		return Result::SUCCESS;
	}

	return Result::GENERIC_ERROR;
}

Result GfxCoreGL::DeleteFence_impl(JseFenceID id)
{
	auto it = fence_map_.find(id);

	if (it == fence_map_.end()) {
#ifdef _DEBUG
		Info("Fence %d not exists", id.internal());
#endif
		return Result::NOT_EXISTS;
	}

	GL_CHECK(glDeleteSync(it->second));
	fence_map_.erase(id);

	return Result::SUCCESS;
}

Result GfxCoreGL::WaitSync_impl(JseFenceID id, uint64_t timeout)
{
	auto it = fence_map_.find(id);

	if (it == fence_map_.end()) {
#ifdef _DEBUG
		Info("Fence %d not exists", id.internal());
#endif
		return Result::NOT_EXISTS;
	}
	if (timeout > 0) {
		GLenum waitReturn = GL_UNSIGNALED;
		int loop{};
		while (waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED)
		{
			waitReturn = glClientWaitSync(it->second, GL_SYNC_FLUSH_COMMANDS_BIT, timeout);
			++loop;
		}
#ifdef _DEBUG
		if (loop > 1) {
			Info("WaitSync loop > 1 (%d)", loop);
		}
#endif
	}
	else {
		glWaitSync(it->second, 0, GL_TIMEOUT_IGNORED);
	}
	return Result::SUCCESS;
}

void GfxCoreGL::BindVertexBuffers_impl(uint32_t firstBinding, uint32_t bindingCount, const JseBufferID* pBuffers, const JseDeviceSize* pOffsets)
{
	if (activePipelineData_.pData) {
		if (bindingCount == 1) {
			BindVertexBuffer_impl(firstBinding, *pBuffers, *pOffsets);
		}
		else {
			vertex_buffer_bindings_.clear();
			vertex_buffer_offsets_.clear();
			vertex_buffer_strides_.clear();

			for (int i = 0; i < bindingCount; ++i) {
				const auto& data = buffer_data_map_.at(pBuffers[i]);
				vertex_buffer_bindings_.push_back(data.buffer);
				vertex_buffer_offsets_.push_back(static_cast<GLintptr>(pOffsets[i]));
				vertex_buffer_strides_.push_back(static_cast<GLsizei>(activePipelineData_.pData->binding[i].stride));
			}

			GL_CHECK(glBindVertexBuffers(firstBinding, bindingCount, vertex_buffer_bindings_.data(), vertex_buffer_offsets_.data(), vertex_buffer_strides_.data()));
		}
	}
}

void GfxCoreGL::BindVertexBuffer_impl(uint32_t binding, JseBufferID buffer, JseDeviceSize offset)
{
	const auto& data = buffer_data_map_.at(buffer);
	if (activePipelineData_.pData) {
		GL_CHECK(glBindVertexBuffer(binding, data.buffer, offset, activePipelineData_.pData->binding[binding].stride));
	}
}

void GfxCoreGL::BindIndexBuffer_impl(JseBufferID buffer, uint32_t offset, JseIndexType type)
{
	const auto& data = buffer_data_map_.at(buffer);
	active_index_offset_ = static_cast<GLintptr>(offset);
	active_index_type_ = type == JseIndexType::UINT16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
	if (stateCache_.indexBuffer != data.buffer) {
		stateCache_.indexBuffer = data.buffer;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.buffer);
	}
}

void GfxCoreGL::Draw_impl(JseTopology mode, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	GL_CHECK(glDrawArraysInstancedBaseInstance(MapJseTopologyGl(mode), firstVertex, vertexCount, instanceCount, firstInstance));
}

void GfxCoreGL::DrawIndexed_impl(JseTopology mode, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	if (activePipelineData_.pData) {
		const uint32_t mult = active_index_type_ == GL_UNSIGNED_SHORT ? 2 : 4;
		GL_CHECK(glDrawElementsInstancedBaseVertexBaseInstance(
			MapJseTopologyGl(mode),
			indexCount,
			active_index_type_,
			RCAST(const void*, (firstIndex * mult)),
			instanceCount,
			vertexOffset,
			firstInstance));
	}
}

void GfxCoreGL::Viewport_impl(const JseRect2D& x)
{
	_glViewport(x.x, x.y, x.w, x.h);
}

void GfxCoreGL::Scissor_impl(const JseRect2D& x)
{
	_glScissor(x.x, x.y, x.w, x.h);
}

void GfxCoreGL::BeginRendering_impl()
{
	SDL_GL_MakeCurrent(windowHandle_, glcontext_);
}

void GfxCoreGL::EndRendering_impl()
{
	SDL_GL_MakeCurrent(windowHandle_, NULL);
}

void GfxCoreGL::SwapChainNextImage_impl()
{
	SDL_GL_SwapWindow(windowHandle_);
}

Result GfxCoreGL::GetDeviceCapabilities_impl(JseDeviceCapabilities& dest)
{
	dest = deviceCapabilities_;

	return Result::SUCCESS;
}

Result GfxCoreGL::SetVSyncInterval_impl(int interval)
{
	if (SDL_GL_SetSwapInterval(interval) == -1) {
		Error("%s", SDL_GetError());
		return Result::GENERIC_ERROR;
	}

	return Result::SUCCESS;
}

Result GfxCoreGL::GetSurfaceDimension_impl(glm::ivec2& x)
{
	//SDL_GetWindowSize(windowHandle_, &_w, &_h);
	SDL_GL_GetDrawableSize(windowHandle_, &x.x, &x.y);

	return Result::SUCCESS;
}

void GfxCoreGL::Shutdown_impl()
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

void GfxCoreGL::SetRenderState(JseRenderState state, bool force)
{
	uint64_t diff = state ^ gl_state_;

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
		switch (state & GLS_CULL_MASK)
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
		switch (state & GLS_DEPTHFUNC_BITS)
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

		switch (state & GLS_SRCBLEND_BITS)
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

		switch (state & GLS_DSTBLEND_BITS)
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
		if (state & GLS_DEPTHMASK)
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
		GLboolean r = (state & GLS_REDMASK) ? GL_FALSE : GL_TRUE;
		GLboolean g = (state & GLS_GREENMASK) ? GL_FALSE : GL_TRUE;
		GLboolean b = (state & GLS_BLUEMASK) ? GL_FALSE : GL_TRUE;
		GLboolean a = (state & GLS_ALPHAMASK) ? GL_FALSE : GL_TRUE;
		glColorMask(r, g, b, a);
	}

	//
	// fill/line mode
	//
	if (diff & GLS_POLYMODE_LINE)
	{
		if (state & GLS_POLYMODE_LINE)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	//
	// polygon position
	//
	if (diff & GLS_POLYGON_OFFSET)
	{
		if (state & GLS_POLYGON_OFFSET)
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
		if ((state & (GLS_STENCIL_FUNC_BITS | GLS_STENCIL_OP_BITS)) != 0)
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
		GLuint ref = GLuint((state & GLS_STENCIL_FUNC_REF_BITS) >> GLS_STENCIL_FUNC_REF_SHIFT);
		GLuint mask = GLuint((state & GLS_STENCIL_FUNC_MASK_BITS) >> GLS_STENCIL_FUNC_MASK_SHIFT);
		GLenum func = 0;

		switch (state & GLS_STENCIL_FUNC_BITS)
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

		switch (state & GLS_STENCIL_OP_FAIL_BITS)
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
		switch (state & GLS_STENCIL_OP_ZFAIL_BITS)
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
		switch (state & GLS_STENCIL_OP_PASS_BITS)
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

	gl_state_ = state;
}

void GfxCoreGL::SetUniforms(DescriptorSetData& set, const JseUniformMap& uniforms)
{
	if (activePipelineData_.pData == nullptr) return;

	GLuint program = activePipelineData_.pData->program;

	GLUniformBinder binder;
	for (const auto& cb : uniforms)
	{
		auto location = set.uniform_location_map.find(cb.first);
		GLint uniform_location;
		if (location != std::end(set.uniform_location_map))
		{
			uniform_location = location->second;
		}
		else {
			GL_CHECK(uniform_location = glGetUniformLocation(program, cb.first.c_str()));
			set.uniform_location_map.emplace(cb.first, uniform_location);
#ifdef _DEBUG
			if (uniform_location == -1) {
				Warning("Uniform variable %s not found!!!", cb.first.c_str());
			}
#endif
		}

		if (uniform_location == -1) continue;

		binder.update(uniform_location, cb.second);

	}
}

void GfxCoreGL::_glBindFramebuffer(GLenum a, GLuint b)
{
	if (stateCache_.framebuffer != b) {
		stateCache_.framebuffer = b;
		GL_CHECK(glBindFramebuffer(a, b));
	}
}

void GfxCoreGL::_glViewport(GLint x, GLint y, GLsizei w, GLsizei h)
{
	if (stateCache_.viewport.x != x || stateCache_.viewport.y != y || stateCache_.viewport.w != w || stateCache_.viewport.h != h) {
		stateCache_.viewport.x = x;
		stateCache_.viewport.y = y;
		stateCache_.viewport.w = w;
		stateCache_.viewport.h = h;
		GL_CHECK(glViewport(x, y, w, h));
	}
}

void GfxCoreGL::_glScissor(GLint x, GLint y, GLsizei w, GLsizei h)
{
	if (stateCache_.scissor.x != x || stateCache_.scissor.y != y || stateCache_.scissor.w != w || stateCache_.scissor.h != h) {
		stateCache_.scissor.x = x;
		stateCache_.scissor.y = y;
		stateCache_.scissor.w = w;
		stateCache_.scissor.h = h;
		GL_CHECK(glScissor(x, y, w, h));
	}

}

void GfxCoreGL::_glScissorEnabled(bool b)
{
	if (scissorEnabled_ != b) {
		scissorEnabled_ = b;
		if (b) glEnable(GL_SCISSOR_TEST);
		else glDisable(GL_SCISSOR_TEST);
	}
}

Result GfxCoreGL::UpdateImageData_mutable(const JseImageUploadInfo& cmd, const ImageData& iData)
{

	if (stateCache_.unpackAlignment != 1) {
		GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
		stateCache_.unpackAlignment = 1;
	}

	bool const compressed = iData.compressed;

	GLenum const binding = MapTexureTargetToBinding(iData.target);
	GLint bound{};

	GL_CHECK(glGetIntegerv(binding, &bound));
	GL_CHECK(glBindTexture(iData.target, iData.texture));
	GLenum const target = (iData.target == GL_TEXTURE_CUBE_MAP) ? GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLint>(cmd.face) : iData.target;
	GLint const depth = (iData.target == GL_TEXTURE_CUBE_MAP_ARRAY ? 6 : 1) * cmd.depth + static_cast<GLint>(cmd.face);

	switch (iData.target) {
	case GL_TEXTURE_1D:
		GL_CHECK(
		compressed
			? glCompressedTexImage1D(target, cmd.level, iData.internal_format, cmd.width, 0, cmd.imageSize, cmd.data)
			: glTexImage1D(target, cmd.level, iData.internal_format, cmd.width, 0, iData.format, iData.type, cmd.data));
		break;
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_2D:
	case GL_TEXTURE_CUBE_MAP:
		GL_CHECK(
		compressed
		? glCompressedTexImage2D(target, cmd.level, iData.internal_format, cmd.width, cmd.height, 0, cmd.imageSize, cmd.data)
		: glTexImage2D(target, cmd.level, iData.internal_format, cmd.width, cmd.height, 0, iData.format, iData.type, cmd.data));
		break;
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_CUBE_MAP_ARRAY:
	case GL_TEXTURE_3D:
		GL_CHECK(
		compressed
			? glCompressedTexImage3D(iData.target, cmd.level, iData.internal_format, cmd.width, cmd.height, depth, 0, cmd.imageSize, cmd.data)
			: glTexImage3D(iData.target, cmd.level, iData.internal_format, cmd.width, cmd.height, depth, 0, iData.format, iData.type, cmd.data));
		break;
	}

	GL_CHECK(glBindTexture(iData.target, bound));

	return Result::SUCCESS;
}

Result GfxCoreGL::UpdateImageData_immutable(const JseImageUploadInfo& cmd, const ImageData& data)
{
	if (stateCache_.unpackAlignment != 1) {
		GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
		stateCache_.unpackAlignment = 1;
	}

	GLuint const zoffset = (data.target == GL_TEXTURE_CUBE_MAP_ARRAY ? 6 : 1) * cmd.zoffset + cmd.face;

	switch (data.target)
	{
	case GL_TEXTURE_1D:
		GL_CHECK(
			data.compressed
			? glCompressedTextureSubImage1D(data.texture, cmd.level, cmd.xoffset, cmd.width, data.format, cmd.imageSize, cmd.data)
			: glTextureSubImage1D(data.texture, cmd.level, cmd.xoffset, cmd.width, data.format, data.type, cmd.data));
		break;
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_2D:
		GL_CHECK(
			data.compressed
			? glCompressedTextureSubImage2D(data.texture, cmd.level, cmd.xoffset, cmd.yoffset, cmd.width, cmd.height, data.format, cmd.imageSize, cmd.data)
			: glTextureSubImage2D(data.texture, cmd.level, cmd.xoffset, cmd.yoffset, cmd.width, cmd.height, data.format, data.type, cmd.data));
		break;
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_CUBE_MAP_ARRAY:
	case GL_TEXTURE_3D:
		GL_CHECK(
		data.compressed
			? glCompressedTextureSubImage3D(data.texture, cmd.level, cmd.xoffset, cmd.yoffset, cmd.zoffset, cmd.width, cmd.height, cmd.depth, data.format, cmd.imageSize, cmd.data)
			: glTextureSubImage3D(data.texture, cmd.level, cmd.xoffset, cmd.yoffset, zoffset, cmd.width, cmd.height, cmd.depth, data.format, data.type, cmd.data));
		break;
	}

	return Result::SUCCESS;
}

Result GfxCoreGL::UpdateImageData_impl(const JseImageUploadInfo& cmd)
{
	auto find = texture_data_map_.find(cmd.imageId);

	if (find == std::end(texture_data_map_)) return Result::NOT_EXISTS;

	auto& iData = find->second;
	
	if (iData.immutable) {
		return UpdateImageData_immutable(cmd, iData);
	}
	else {
		return UpdateImageData_mutable(cmd, iData);
	}
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

	static GLenum MapJseImageTargetGl(JseImageTarget t) {
		switch (t) {
		case JseImageTarget::D1:
			return GL_TEXTURE_1D;
		case JseImageTarget::D1_ARRAY:
			return GL_TEXTURE_1D_ARRAY;
		case JseImageTarget::D2:
			return GL_TEXTURE_2D;
		case JseImageTarget::D2_ARRAY:
			return GL_TEXTURE_2D_ARRAY;
		case JseImageTarget::D3:
			return GL_TEXTURE_3D;
		case JseImageTarget::D3_ARRAY:
			return 0;
		case JseImageTarget::CUBEMAP:
			return GL_TEXTURE_CUBE_MAP;
		case JseImageTarget::CUBEMAP_ARRAY:
			return GL_TEXTURE_CUBE_MAP_ARRAY;
		default: 
			return 0;
		}
	}

	static GLenum MapJseBufferTargetGl(JseBufferTarget t) {
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

	static GLenum MapJseFilterGl(JseFilter t) {
		switch (t) {
		case JseFilter::LINEAR:
			return GL_LINEAR;
		case JseFilter::NEAREST:
			return GL_NEAREST;
		case JseFilter::NEAREST_MIPMAP_NEAREST:
			return GL_NEAREST_MIPMAP_NEAREST;
		case JseFilter::LINEAR_MIPMAP_NEAREST:
			return GL_LINEAR_MIPMAP_NEAREST;
		case JseFilter::NEAREST_MIPMAP_LINEAR:
			return GL_NEAREST_MIPMAP_LINEAR;
		case JseFilter::LINEAR_MIPMAP_LINEAR:
			return GL_LINEAR_MIPMAP_LINEAR;
		}
	}

	static GLenum MapJseTilingGl(JseImageTiling t) {
		switch (t) {
		case JseImageTiling::REPEAT:
			return GL_REPEAT;
		case JseImageTiling::CLAMP_TO_EDGE:
			return GL_CLAMP_TO_EDGE;
		case JseImageTiling::CLAMP_TO_BORDER:
			return GL_CLAMP_TO_BORDER;
		case JseImageTiling::MIRRORED_REPEAT:
			return GL_MIRRORED_REPEAT;
		}
	}

	static GLenum MapJseCubemapFaceGl(JseCubeMapFace t) {
		return GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<int>(t);
	}

	static GLenum MapJseShaderStageGl(JseShaderStage t) {
		switch (t) {
		case JseShaderStage::VERTEX:
			return GL_VERTEX_SHADER;
		case JseShaderStage::COMPUTE:
			return GL_COMPUTE_SHADER;
		case JseShaderStage::FRAGMENT:
			return GL_FRAGMENT_SHADER;
		case JseShaderStage::GEOMETRY:
			return GL_GEOMETRY_SHADER;
		case JseShaderStage::TESSELATION_CONTROL:
			return GL_TESS_CONTROL_SHADER;
		case JseShaderStage::TESSELATION:
			return GL_TESS_EVALUATION_SHADER;
		default:
			return 0;
		}
	}

	static GLenum MapJseAccessImageAccessGl(JseAccess t) {
		switch (t) {
		case JseAccess::READ: return GL_READ_ONLY;
		case JseAccess::WRITE: return GL_WRITE_ONLY;
		case JseAccess::READ_WRITE: return GL_READ_WRITE;
		}
	}

	static GLenum MapJseTopologyGl(JseTopology p)
	{
		switch (p)
		{
		case JseTopology::Lines:
			return GL_LINES;
		case JseTopology::LineLoop:
			return GL_LINE_LOOP;
		case JseTopology::LineStrip:
			return GL_LINE_STRIP;
		case JseTopology::Point:
			return GL_POINTS;
		case JseTopology::Triangles:
			return GL_TRIANGLES;
		case JseTopology::TriangleFan:
			return GL_TRIANGLE_FAN;
		case JseTopology::TriangleStrip:
			return GL_TRIANGLE_STRIP;
		}
	}
	static JseFormat MapGlFormatToJse(GLenum format) {
		
	}
	static GLenum MapTexureTargetToBinding(GLenum target) {
		switch (target)
		{
		case GL_TEXTURE_1D:
			return GL_TEXTURE_BINDING_1D;
		case GL_TEXTURE_1D_ARRAY:
			return GL_TEXTURE_BINDING_1D_ARRAY;
		case GL_TEXTURE_2D:
			return GL_TEXTURE_BINDING_2D;
		case GL_TEXTURE_2D_ARRAY:
			return GL_TEXTURE_BINDING_2D_ARRAY;
		case GL_TEXTURE_3D:
			return GL_TEXTURE_BINDING_3D;
		case GL_TEXTURE_CUBE_MAP:
			return GL_TEXTURE_BINDING_CUBE_MAP;
		case GL_TEXTURE_CUBE_MAP_ARRAY:
			return GL_TEXTURE_BINDING_CUBE_MAP_ARRAY;

		}
	}