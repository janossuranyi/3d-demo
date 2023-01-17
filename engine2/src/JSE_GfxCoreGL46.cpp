#include <GL/glew.h>
#include "JSE.h"
#include "JSE_GfxCoreGL46.h"

GLenum MapJseBufferTargetGl(JseBufferTarget t);
GLenum MapJseFilterGl(JseFilter t);
GLenum MapJseTilingGl(JseImageTiling t);
GLenum MapJseImageTargetGl(JseImageTarget t);
GLenum MapJseCubemapFaceGl(JseCubeMapFace t);

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


TextureFormatInfo s_texture_format[] = {
	{GL_ALPHA,              GL_ZERO,         GL_ALPHA,            GL_UNSIGNED_BYTE,                false,  1}, // A8
	{GL_R8,                 GL_ZERO,         GL_RED,              GL_UNSIGNED_BYTE,                false,  1}, // R8
	{GL_R8I,                GL_ZERO,         GL_RED,              GL_BYTE,                         false,  1}, // R8I
	{GL_R8UI,               GL_ZERO,         GL_RED,              GL_UNSIGNED_BYTE,                false,  1}, // R8U
	{GL_R8_SNORM,           GL_ZERO,         GL_RED,              GL_BYTE,                         false,  1}, // R8S
	{GL_R16,                GL_ZERO,         GL_RED,              GL_UNSIGNED_SHORT,               false,  1}, // R16
	{GL_R16I,               GL_ZERO,         GL_RED,              GL_SHORT,                        false,  1}, // R16I
	{GL_R16UI,              GL_ZERO,         GL_RED,              GL_UNSIGNED_SHORT,               false,  1}, // R16U
	{GL_R16F,               GL_ZERO,         GL_RED,              GL_HALF_FLOAT,                   false,  1}, // R16F
	{GL_R16_SNORM,          GL_ZERO,         GL_RED,              GL_SHORT,                        false,  1}, // R16S
	{GL_R32I,               GL_ZERO,         GL_RED,              GL_INT,                          false,  1}, // R32I
	{GL_R32UI,              GL_ZERO,         GL_RED,              GL_UNSIGNED_INT,                 false,  1}, // R32U
	{GL_R32F,               GL_ZERO,         GL_RED,              GL_FLOAT,                        false,  1}, // R32F
	{GL_RG8,                GL_ZERO,         GL_RG,               GL_UNSIGNED_BYTE,                false,  2}, // RG8
	{GL_RG8I,               GL_ZERO,         GL_RG,               GL_BYTE,                         false,  2}, // RG8I
	{GL_RG8UI,              GL_ZERO,         GL_RG,               GL_UNSIGNED_BYTE,                false,  2}, // RG8U
	{GL_RG8_SNORM,          GL_ZERO,         GL_RG,               GL_BYTE,                         false,  2}, // RG8S
	{GL_RG16,               GL_ZERO,         GL_RG,               GL_UNSIGNED_SHORT,               false,  2}, // RG16
	{GL_RG16I,              GL_ZERO,         GL_RG,               GL_SHORT,                        false,  2}, // RG16I
	{GL_RG16UI,             GL_ZERO,         GL_RG,               GL_UNSIGNED_SHORT,               false,  2}, // RG16U
	{GL_RG16F,              GL_ZERO,         GL_RG,               GL_FLOAT,                        false,  2}, // RG16F
	{GL_RG16_SNORM,         GL_ZERO,         GL_RG,               GL_SHORT,                        false,  2}, // RG16S
	{GL_RG32I,              GL_ZERO,         GL_RG,               GL_INT,                          false,  2}, // RG32I
	{GL_RG32UI,             GL_ZERO,         GL_RG,               GL_UNSIGNED_INT,                 false,  2}, // RG32U
	{GL_RG32F,              GL_ZERO,         GL_RG,               GL_FLOAT,                        false,  2}, // RG32F
	{GL_RGB8,               GL_SRGB8,        GL_RGB,              GL_UNSIGNED_BYTE,                false,  3}, // RGB8
	{GL_RGB8I,              GL_ZERO,         GL_RGB,              GL_BYTE,                         false,  3}, // RGB8I
	{GL_RGB8UI,             GL_ZERO,         GL_RGB,              GL_UNSIGNED_BYTE,                false,  3}, // RGB8U
	{GL_RGB8_SNORM,         GL_ZERO,         GL_RGB,              GL_BYTE,                         false,  3}, // RGB8S
	{GL_RGBA8,              GL_SRGB8_ALPHA8, GL_BGRA,             GL_UNSIGNED_BYTE,                false,  4}, // BGRA8
	{GL_RGBA8,              GL_SRGB8_ALPHA8, GL_RGBA,             GL_UNSIGNED_BYTE,                false,  4}, // RGBA8
	{GL_RGBA8I,             GL_ZERO,         GL_RGBA,             GL_BYTE,                         false,  4}, // RGBA8I
	{GL_RGBA8UI,            GL_ZERO,         GL_RGBA,             GL_UNSIGNED_BYTE,                false,  4}, // RGBA8U
	{GL_RGBA8_SNORM,        GL_ZERO,         GL_RGBA,             GL_BYTE,                         false,  4}, // RGBA8S
	{GL_RGBA16,             GL_ZERO,         GL_RGBA,             GL_UNSIGNED_SHORT,               false,  4}, // RGBA16
	{GL_RGBA16I,            GL_ZERO,         GL_RGBA,             GL_SHORT,                        false,  4}, // RGBA16I
	{GL_RGBA16UI,           GL_ZERO,         GL_RGBA,             GL_UNSIGNED_SHORT,               false,  4}, // RGBA16U
	{GL_RGBA16F,            GL_ZERO,         GL_RGBA,             GL_HALF_FLOAT,                   false,  4}, // RGBA16F
	{GL_RGBA16_SNORM,       GL_ZERO,         GL_RGBA,             GL_SHORT,                        false,  4}, // RGBA16S
	{GL_RGBA32I,            GL_ZERO,         GL_RGBA,             GL_INT,                          false,  4}, // RGBA32I
	{GL_RGBA32UI,           GL_ZERO,         GL_RGBA,             GL_UNSIGNED_INT,                 false,  4}, // RGBA32U
	{GL_RGBA32F,            GL_ZERO,         GL_RGBA,             GL_FLOAT,                        false,  4}, // RGBA32F
	{GL_RGBA4,              GL_ZERO,         GL_RGBA,             GL_UNSIGNED_SHORT_4_4_4_4_REV,   false,  4}, // RGBA4
	{GL_RGB5_A1,            GL_ZERO,         GL_RGBA,             GL_UNSIGNED_SHORT_1_5_5_5_REV,   false,  4}, // RGB5A1
	{GL_RGB10_A2,           GL_ZERO,         GL_RGBA,             GL_UNSIGNED_INT_2_10_10_10_REV,  false,  4}, // RGB10A2
	{GL_R11F_G11F_B10F,     GL_ZERO,         GL_RGB,              GL_UNSIGNED_INT_10F_11F_11F_REV, false,  3}, // RG11B10F
	{GL_COMPRESSED_RGB,     GL_COMPRESSED_SRGB,GL_RGB,            GL_UNSIGNED_BYTE,                false,  3}, // RGB8_COMPRESSED
	{GL_COMPRESSED_RGBA,    GL_COMPRESSED_SRGB_ALPHA,GL_RGBA,     GL_UNSIGNED_BYTE,                false,  4}, // RGBA8_COMPRESSED
	{GL_DEPTH_COMPONENT16,  GL_ZERO,         GL_DEPTH_COMPONENT,  GL_UNSIGNED_SHORT,               false,  1}, // D16
	{GL_DEPTH_COMPONENT24,  GL_ZERO,         GL_DEPTH_COMPONENT,  GL_UNSIGNED_INT,                 false,  1}, // D24
	{GL_DEPTH24_STENCIL8,   GL_ZERO,         GL_DEPTH_STENCIL,    GL_UNSIGNED_INT_24_8,            false,  1}, // D24S8
	{GL_DEPTH_COMPONENT32,  GL_ZERO,         GL_DEPTH_COMPONENT,  GL_UNSIGNED_INT,                 false,  1}, // D32
	{GL_DEPTH_COMPONENT32F, GL_ZERO,         GL_DEPTH_COMPONENT,  GL_FLOAT,                        false,  1}, // D16F
	{GL_DEPTH_COMPONENT32F, GL_ZERO,         GL_DEPTH_COMPONENT,  GL_FLOAT,                        false,  1}, // D24F
	{GL_DEPTH_COMPONENT32F, GL_ZERO,         GL_DEPTH_COMPONENT,  GL_FLOAT,                        false,  1}, // D32F
	{GL_STENCIL_INDEX8,     GL_ZERO,         GL_STENCIL_INDEX,    GL_UNSIGNED_BYTE,                false,  1}, // D0S8
};

JseGfxCoreGL::~JseGfxCoreGL()
{
	Shutdown_impl();
}

JseGfxCoreGL::JseGfxCoreGL() : 
	windowHandle_(nullptr),
	useDebugMode_(false),
	glcontext_(0),
	glVersion_(0)
{
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

	deviceCapabilities_.renderer = (const char*)glGetString(GL_RENDERER);
	deviceCapabilities_.rendererVersion = (const char*)glGetString(GL_VERSION);
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &deviceCapabilities_.maxArrayTextureLayers);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &deviceCapabilities_.maxTextureImageUnits);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &deviceCapabilities_.maxTextureSize);
	glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &deviceCapabilities_.maxComputeSharedMemorySize);
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &deviceCapabilities_.maxUniformBlockSize);
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &deviceCapabilities_.maxShaderStorageBlockSize);

	glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &deviceCapabilities_.availableVideoMemory);
	glGetError();
	glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, &deviceCapabilities_.availableVideoMemory);
	glGetError();


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

	if (!flags) {
		flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT;
	}

	const GLsizeiptr _size = static_cast<GLsizeiptr>(cmd.size);

	GL_CHECK(glCreateBuffers(1, &bufferData.buffer));
	GL_CHECK(glNamedBufferStorage(bufferData.buffer, _size, nullptr, flags));

	if (cmd.storageFlags & JSE_BUFFER_STORAGE_PERSISTENT_BIT) {
		GL_CHECK(bufferData.mapptr = glMapNamedBufferRange(bufferData.buffer, 0, _size, flags));
	}

	bufferData.size = _size;
	bufferData.target = MapJseBufferTargetGl(cmd.target);

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

	if (find != std::end(texture_data_map_)) {
		return JseResult::ALREADY_EXISTS;
	}

	Info("Format index: %d", static_cast<size_t>(cmd.format));

	ImageData data{};
	TextureFormatInfo formatInfo = s_texture_format[static_cast<size_t>(cmd.format)];

	data.target = MapJseImageTargetGl(cmd.target);
	if (data.target == 0) {
		return JseResult::INVALID_VALUE;
	}

	GLenum const internalFormat = cmd.srgb ? formatInfo.internal_format_srgb : formatInfo.internal_format;
	
	data.format = formatInfo.format;
	data.type = formatInfo.type;
	
	GL_CHECK(glCreateTextures(data.target, 1, &data.texture));
	if (cmd.target == JseImageTarget::D1) {
		GL_CHECK(glTextureStorage1D(data.texture, cmd.levelCount, internalFormat, cmd.width));
	}
	else if (cmd.target == JseImageTarget::D2 || cmd.target == JseImageTarget::CUBEMAP || cmd.target == JseImageTarget::D1_ARRAY) {
		GL_CHECK(glTextureStorage2D(data.texture, cmd.levelCount, internalFormat, cmd.width, cmd.height));
	}
	else if (cmd.target == JseImageTarget::D3 || cmd.target == JseImageTarget::D2_ARRAY || cmd.target == JseImageTarget::CUBEMAP_ARRAY) {
		GLint depth = cmd.target == JseImageTarget::CUBEMAP_ARRAY ? 6 * cmd.depth : cmd.depth;
		GL_CHECK(glTextureStorage3D(data.texture, cmd.levelCount, internalFormat, cmd.width, cmd.height, depth));
	}

	GLenum tilingS = MapJseTilingGl(JseImageTiling::CLAMP_TO_EDGE);
	GLenum tilingT = MapJseTilingGl(JseImageTiling::CLAMP_TO_EDGE);
	GLenum tilingR = MapJseTilingGl(JseImageTiling::CLAMP_TO_EDGE);
	GLenum minFilter = MapJseFilterGl(JseFilter::LINEAR);
	GLenum magFilter = MapJseFilterGl(JseFilter::LINEAR);
	GLfloat maxAnisotropy = 1.0f;
	GLfloat minLod = 0.0f;
	GLfloat maxLod = 1000.0f;
	GLfloat lodBias = 0.0f;

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
	}

	// glTextureParameterf ...
	GL_CHECK(glTextureParameteri(data.texture, GL_TEXTURE_MIN_FILTER, minFilter));
	GL_CHECK(glTextureParameteri(data.texture, GL_TEXTURE_MAG_FILTER, magFilter));
	GL_CHECK(glTextureParameteri(data.texture, GL_TEXTURE_WRAP_S, tilingS));
	GL_CHECK(glTextureParameteri(data.texture, GL_TEXTURE_WRAP_T, tilingT));
	GL_CHECK(glTextureParameteri(data.texture, GL_TEXTURE_WRAP_R, tilingR));
	GL_CHECK(glTextureParameterf(data.texture, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy));
	GL_CHECK(glTextureParameterf(data.texture, GL_TEXTURE_MIN_LOD, minLod));
	GL_CHECK(glTextureParameterf(data.texture, GL_TEXTURE_MAX_LOD, maxLod));
	GL_CHECK(glTextureParameterf(data.texture, GL_TEXTURE_LOD_BIAS, lodBias));

	texture_data_map_.emplace(cmd.imageId, data);

	return JseResult::SUCCESS;
}

JseResult JseGfxCoreGL::UpdateImageData_impl(const JseImageUploadInfo& cmd)
{
	auto find = texture_data_map_.find(cmd.imageId);

	if (find == std::end(texture_data_map_)) return JseResult::NOT_EXISTS;
	
	auto& iData = find->second;

	if (stateCache_.unpackAlignment != 1) {
		GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
		stateCache_.unpackAlignment = 1;
	}

	if (iData.target == GL_TEXTURE_1D) {
		GL_CHECK(glTextureSubImage1D(iData.texture, cmd.level, cmd.xoffset, cmd.width, iData.format, iData.type, cmd.data.data()));
	}
	else if (iData.target == GL_TEXTURE_2D || iData.target == GL_TEXTURE_1D_ARRAY) {
		GL_CHECK(glTextureSubImage2D(iData.texture, cmd.level, cmd.xoffset, cmd.yoffset, cmd.width, cmd.height, iData.format, iData.type, cmd.data.data()));
	}
	else if (iData.target == GL_TEXTURE_3D || iData.target == GL_TEXTURE_2D_ARRAY || iData.target == GL_TEXTURE_CUBE_MAP || iData.target == GL_TEXTURE_CUBE_MAP_ARRAY) {
		/*
		* GL_TEXTURE_CUBE_MAP_ARRAY
		* Layers also have to be dealt with in an unusual way.
		* Cubemap array textures have, for each mipmap, some number of cubemaps.
		* That number of cubemaps is the number of layers.
		* But since each cubemap is composed of 6 2D faces, array cubemaps also have a number of layer-faces, which is 6 times the number of layers.
		* Some interfaces count by layers, and others count by layer-faces.
		* Every OpenGL API call that operates on cubemap array textures takes layer-faces, not array layers. For example, when you allocate storage for the texture, you would use glTexStorage3D or glTexImage3D or similar. However, the depth​ parameter will be the number of layer-faces, not layers. So it must be divisible by 6.
		* Similarly, when uploading texel data to the cubemap array, the parameters that represent the Z component are layer-faces.
		* So if you want to upload to just the positive Z face of the second layer in the array, you would use call glTexSubImage3D,
		* with the zoffset​​ parameter set to 10 (layer 1 * 6 faces per layer + face index 4),
		* and the depth​ set to 1 (because you're only uploading one layer-face).
		*/
		GLint const zoffset = (iData.target == GL_TEXTURE_CUBE_MAP_ARRAY ? 6 : 1) * cmd.zoffset + static_cast<GLint>(cmd.face);
		GL_CHECK(glTextureSubImage3D(iData.texture, cmd.level, cmd.xoffset, cmd.yoffset, zoffset, cmd.width, cmd.height, cmd.depth, iData.format, iData.type, cmd.data.data()));
	}

	return JseResult::SUCCESS;
}

JseResult JseGfxCoreGL::CreateGraphicsPipeline_impl(const JseGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo)
{
    return JseResult();
}

JseResult JseGfxCoreGL::GetDeviceCapabilities_impl(JseDeviceCapabilities& dest)
{
	dest = deviceCapabilities_;

	return JseResult::SUCCESS;
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