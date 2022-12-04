#include "common.h"
#include "gfx/handle.h"
#include "gfx/vertex.h"
#include "gfx/opengl/gl_context.h"
#include "gfx/opengl/gl_helper.h"

#include "logger.h"

namespace gfx {

	TextureFormatInfo s_texture_format[] = {
		{GL_ALPHA,              GL_ZERO,         GL_ALPHA,            GL_UNSIGNED_BYTE,                false,  1}, // A8
		{GL_R8,                 GL_ZERO,         GL_RED,              GL_UNSIGNED_BYTE,                false,  1}, // R8
		{GL_R8I,                GL_ZERO,         GL_RED,              GL_BYTE,                         false,  1}, // R8I
		{GL_R8UI,               GL_ZERO,         GL_RED,              GL_UNSIGNED_BYTE,                false,  1}, // R8U
		{GL_R8_SNORM,           GL_ZERO,         GL_RED,              GL_BYTE,                         false,  1}, // R8S
		{GL_R16,                GL_ZERO,         GL_RED,              GL_UNSIGNED_SHORT,               false,  2}, // R16
		{GL_R16I,               GL_ZERO,         GL_RED,              GL_SHORT,                        false,  2}, // R16I
		{GL_R16UI,              GL_ZERO,         GL_RED,              GL_UNSIGNED_SHORT,               false,  2}, // R16U
		{GL_R16F,               GL_ZERO,         GL_RED,              GL_HALF_FLOAT,                   false,  2}, // R16F
		{GL_R16_SNORM,          GL_ZERO,         GL_RED,              GL_SHORT,                        false,  2}, // R16S
		{GL_R32I,               GL_ZERO,         GL_RED,              GL_INT,                          false,  4}, // R32I
		{GL_R32UI,              GL_ZERO,         GL_RED,              GL_UNSIGNED_INT,                 false,  4}, // R32U
		{GL_R32F,               GL_ZERO,         GL_RED,              GL_FLOAT,                        false,  4}, // R32F
		{GL_RG8,                GL_ZERO,         GL_RG,               GL_UNSIGNED_BYTE,                false,  2}, // RG8
		{GL_RG8I,               GL_ZERO,         GL_RG,               GL_BYTE,                         false,  2}, // RG8I
		{GL_RG8UI,              GL_ZERO,         GL_RG,               GL_UNSIGNED_BYTE,                false,  2}, // RG8U
		{GL_RG8_SNORM,          GL_ZERO,         GL_RG,               GL_BYTE,                         false,  2}, // RG8S
		{GL_RG16,               GL_ZERO,         GL_RG,               GL_UNSIGNED_SHORT,               false,  4}, // RG16
		{GL_RG16I,              GL_ZERO,         GL_RG,               GL_SHORT,                        false,  4}, // RG16I
		{GL_RG16UI,             GL_ZERO,         GL_RG,               GL_UNSIGNED_SHORT,               false,  4}, // RG16U
		{GL_RG16F,              GL_ZERO,         GL_RG,               GL_FLOAT,                        false,  8}, // RG16F
		{GL_RG16_SNORM,         GL_ZERO,         GL_RG,               GL_SHORT,                        false,  4}, // RG16S
		{GL_RG32I,              GL_ZERO,         GL_RG,               GL_INT,                          false,  8}, // RG32I
		{GL_RG32UI,             GL_ZERO,         GL_RG,               GL_UNSIGNED_INT,                 false,  2}, // RG32U
		{GL_RG32F,              GL_ZERO,         GL_RG,               GL_FLOAT,                        false,  8}, // RG32F
		{GL_RGB8,               GL_SRGB8,        GL_RGB,              GL_UNSIGNED_BYTE,                false,  3}, // RGB8
		{GL_RGB8I,              GL_ZERO,         GL_RGB,              GL_BYTE,                         false,  3}, // RGB8I
		{GL_RGB8UI,             GL_ZERO,         GL_RGB,              GL_UNSIGNED_BYTE,                false,  3}, // RGB8U
		{GL_RGB8_SNORM,         GL_ZERO,         GL_RGB,              GL_BYTE,                         false,  3}, // RGB8S
		{GL_RGBA8,              GL_SRGB8_ALPHA8, GL_BGRA,             GL_UNSIGNED_BYTE,                false,  4}, // BGRA8
		{GL_RGBA8,              GL_SRGB8_ALPHA8, GL_RGBA,             GL_UNSIGNED_BYTE,                false,  4}, // RGBA8
		{GL_RGBA8I,             GL_ZERO,         GL_RGBA,             GL_BYTE,                         false,  4}, // RGBA8I
		{GL_RGBA8UI,            GL_ZERO,         GL_RGBA,             GL_UNSIGNED_BYTE,                false,  4}, // RGBA8U
		{GL_RGBA8_SNORM,        GL_ZERO,         GL_RGBA,             GL_BYTE,                         false,  4}, // RGBA8S
		{GL_RGBA16,             GL_ZERO,         GL_RGBA,             GL_UNSIGNED_SHORT,               false,  8}, // RGBA16
		{GL_RGBA16I,            GL_ZERO,         GL_RGBA,             GL_SHORT,                        false,  8}, // RGBA16I
		{GL_RGBA16UI,           GL_ZERO,         GL_RGBA,             GL_UNSIGNED_SHORT,               false,  8}, // RGBA16U
		{GL_RGBA16F,            GL_ZERO,         GL_RGBA,             GL_HALF_FLOAT,                   false,  8}, // RGBA16F
		{GL_RGBA16_SNORM,       GL_ZERO,         GL_RGBA,             GL_SHORT,                        false,  8}, // RGBA16S
		{GL_RGBA32I,            GL_ZERO,         GL_RGBA,             GL_INT,                          false, 16}, // RGBA32I
		{GL_RGBA32UI,           GL_ZERO,         GL_RGBA,             GL_UNSIGNED_INT,                 false, 16}, // RGBA32U
		{GL_RGBA32F,            GL_ZERO,         GL_RGBA,             GL_FLOAT,                        false, 16}, // RGBA32F
		{GL_RGBA4,              GL_ZERO,         GL_RGBA,             GL_UNSIGNED_SHORT_4_4_4_4_REV,   false,  2}, // RGBA4
		{GL_RGB5_A1,            GL_ZERO,         GL_RGBA,             GL_UNSIGNED_SHORT_1_5_5_5_REV,   false,  2}, // RGB5A1
		{GL_RGB10_A2,           GL_ZERO,         GL_RGBA,             GL_UNSIGNED_INT_2_10_10_10_REV,  false,  4}, // RGB10A2
		{GL_R11F_G11F_B10F,     GL_ZERO,         GL_RGB,              GL_UNSIGNED_INT_10F_11F_11F_REV, false,  4}, // RG11B10F
		{GL_COMPRESSED_RGB,     GL_COMPRESSED_SRGB,GL_RGB,            GL_UNSIGNED_BYTE,                false,  3}, // RGB8_COMPRESSED
		{GL_COMPRESSED_RGBA,    GL_COMPRESSED_SRGB_ALPHA,GL_RGBA,     GL_UNSIGNED_BYTE,                false,  4}, // RGBA8_COMPRESSED
		{GL_DEPTH_COMPONENT16,  GL_ZERO,         GL_DEPTH_COMPONENT,  GL_UNSIGNED_SHORT,               false,  2}, // D16
		{GL_DEPTH_COMPONENT24,  GL_ZERO,         GL_DEPTH_COMPONENT,  GL_UNSIGNED_INT,                 false,  4}, // D24
		{GL_DEPTH24_STENCIL8,   GL_ZERO,         GL_DEPTH_STENCIL,    GL_UNSIGNED_INT_24_8,            false,  4}, // D24S8
		{GL_DEPTH_COMPONENT32,  GL_ZERO,         GL_DEPTH_COMPONENT,  GL_UNSIGNED_INT,                 false,  4}, // D32
		{GL_DEPTH_COMPONENT32F, GL_ZERO,         GL_DEPTH_COMPONENT,  GL_FLOAT,                        false,  4}, // D16F
		{GL_DEPTH_COMPONENT32F, GL_ZERO,         GL_DEPTH_COMPONENT,  GL_FLOAT,                        false,  4}, // D24F
		{GL_DEPTH_COMPONENT32F, GL_ZERO,         GL_DEPTH_COMPONENT,  GL_FLOAT,                        false,  4}, // D32F
		{GL_STENCIL_INDEX8,     GL_ZERO,         GL_STENCIL_INDEX,    GL_UNSIGNED_BYTE,                false,  1}, // D0S8
	};

	const std::unordered_map<BlendEquation, GLenum> s_blend_equation_map = {
		{BlendEquation::Add, GL_FUNC_ADD},
		{BlendEquation::Subtract, GL_FUNC_SUBTRACT},
		{BlendEquation::ReverseSubtract, GL_FUNC_REVERSE_SUBTRACT},
		{BlendEquation::Min, GL_MIN},
		{BlendEquation::Max, GL_MAX}
	};
	const std::unordered_map<BlendFunc, GLenum> s_blend_func_map = {
		{BlendFunc::Zero, GL_ZERO},
		{BlendFunc::One, GL_ONE},
		{BlendFunc::SrcColor, GL_SRC_COLOR},
		{BlendFunc::OneMinusSrcColor, GL_ONE_MINUS_SRC_COLOR},
		{BlendFunc::DstColor, GL_DST_COLOR},
		{BlendFunc::OneMinusDstColor, GL_ONE_MINUS_DST_COLOR},
		{BlendFunc::SrcAlpha, GL_SRC_ALPHA},
		{BlendFunc::OneMinusSrcAlpha, GL_ONE_MINUS_SRC_ALPHA},
		{BlendFunc::DstAlpha, GL_DST_ALPHA},
		{BlendFunc::OneMinusDstAlpha, GL_ONE_MINUS_DST_ALPHA},
		{BlendFunc::ConstantColor, GL_CONSTANT_COLOR},
		{BlendFunc::OneMinusConstantColor, GL_ONE_MINUS_CONSTANT_COLOR},
		{BlendFunc::ConstantAlpha, GL_CONSTANT_ALPHA},
		{BlendFunc::OneMinusConstantAlpha, GL_ONE_MINUS_CONSTANT_ALPHA},
		{BlendFunc::SrcAlphaSaturate, GL_SRC_ALPHA_SATURATE},
	};
	const std::unordered_map<TextureWrap, GLenum> s_texture_wrap_map = {
		{TextureWrap::ClampToBorder, GL_CLAMP_TO_BORDER},
		{TextureWrap::ClampToEdge, GL_CLAMP_TO_EDGE},
		{TextureWrap::MirroredRepeat, GL_MIRRORED_REPEAT},
		{TextureWrap::Repeat, GL_REPEAT}
	};

	const std::unordered_map<TextureFilter, GLenum> s_texture_filter_map = {
		{TextureFilter::Linear, GL_LINEAR},
		{TextureFilter::LinearNearest, GL_LINEAR_MIPMAP_NEAREST},
		{TextureFilter::Nearest, GL_NEAREST},
		{TextureFilter::NearestLinear, GL_NEAREST_MIPMAP_LINEAR},
		{TextureFilter::LinearLinear, GL_LINEAR_MIPMAP_LINEAR}
	};

	class VertexAttribSetter {
	public:
		VertexAttribSetter() : attrib_index_(0) {}
		~VertexAttribSetter() = default;

		void operator()(const int& value) {
			GL_CHECK(glVertexAttribI1i(attrib_index_, value));
		}
		void operator()(const uint& value) {
			GL_CHECK(glVertexAttribI1ui(attrib_index_, value));
		}
		void operator()(const float& value) {
			GL_CHECK(glVertexAttrib1f(attrib_index_, value));
		}
		void operator()(const ivec2& value) {
			GL_CHECK(glVertexAttribI2iv(attrib_index_, &value[0]));
		}
		void operator()(const ivec3& value) {
			GL_CHECK(glVertexAttribI3iv(attrib_index_, &value[0]));
		}
		void operator()(const ivec4& value) {
			GL_CHECK(glVertexAttribI4iv(attrib_index_, &value[0]));
		}
		void operator()(const vec2& value) {
			GL_CHECK(glVertexAttrib2fv(attrib_index_, &value[0]));
		}
		void operator()(const vec3& value) {
			GL_CHECK(glVertexAttrib3fv(attrib_index_, &value[0]));
		}
		void operator()(const vec4& value) {
			GL_CHECK(glVertexAttrib4fv(attrib_index_, &value[0]));
		}

		void update(GLint index, const VertexAttribData& value)
		{
			attrib_index_ = index;
			std::visit(*this, value);
		}

	private:
		GLuint attrib_index_;
	};

	class UniformBinder {
	public:
		UniformBinder() : uniform_location_{ 0 } {
		}

		~UniformBinder() = default;

		void operator()(const int& value) {
			GL_CHECK(glUniform1i(uniform_location_, value));
		}
		void operator()(const float& value) {
			GL_CHECK(glUniform1f(uniform_location_, value));
		}
		void operator()(const ivec2& value) {
			GL_CHECK(glUniform2iv(uniform_location_, 1, &value[0]));
		}
		void operator()(const ivec3& value) {
			GL_CHECK(glUniform3iv(uniform_location_, 1, &value[0]));
		}
		void operator()(const ivec4& value) {
			GL_CHECK(glUniform4iv(uniform_location_, 1, &value[0]));
		}
		void operator()(const vec2& value) {
			GL_CHECK(glUniform2fv(uniform_location_, 1, &value[0]));
		}
		void operator()(const vec3& value) {
			GL_CHECK(glUniform3fv(uniform_location_, 1, &value[0]));
		}
		void operator()(const vec4& value) {
			GL_CHECK(glUniform4fv(uniform_location_, 1, &value[0]));
		}
		void operator()(const mat3& value) {
			GL_CHECK(glUniformMatrix3fv(uniform_location_, 1, GL_FALSE, &value[0][0]));
		}
		void operator()(const mat4& value) {
			GL_CHECK(glUniformMatrix4fv(uniform_location_, 1, GL_FALSE, &value[0][0]));
		}
		void operator()(const Vector<float>& value) {
			glUniform1fv(uniform_location_, value.size(), (const GLfloat*)value.data());
		}
		void operator()(const Vector<vec4>& value) {
			glUniform4fv(uniform_location_, value.size(), (const GLfloat*)value.data());
		}
		void update(GLint location, const UniformData& value) {
			uniform_location_ = location;
			std::visit(*this, value);
		}
	private:
		GLint uniform_location_;
	};

	int OpenGLRenderContext::red_bits() const {
		return window_.redBits;
	}

	int OpenGLRenderContext::green_bits() const
	{
		return window_.greenBits;
	}

	int OpenGLRenderContext::blue_bits() const
	{
		return window_.blueBits;
	}

	int OpenGLRenderContext::depth_bits() const
	{
		return window_.depthBits;
	}

	int OpenGLRenderContext::stencil_bits() const
	{
		return window_.stencilBits;
	}


	void OpenGLRenderContext::operator()(const cmd::CreateFence& cmd)
	{
		GL_CHECK(fence_map_[cmd.handle] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
	}

	void OpenGLRenderContext::operator()(const cmd::DeleteFence& cmd)
	{
		GLsync sync = fence_map_.at(cmd.handle);

		GL_CHECK(glDeleteSync(sync));

		fence_map_.erase(cmd.handle);
	}

	bool OpenGLRenderContext::create_window(uint16_t w, uint16_t h, bool fullscreen, const std::string& name) {
		int err;

		if ((err = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0)) {
			Error("ERROR: %s", SDL_GetError());
			return false;
		}

		Info("SDL_Init done");

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
#ifdef _DEBUG
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
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

		Info("Using GLEW %s", glewGetString(GLEW_VERSION));

		SDL_GL_SetSwapInterval(1);

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

		gl_version_430_ = glVersion_ >= 430;
		gl_version_440_ = glVersion_ >= 440;
		gl_version_450_ = glVersion_ >= 450;

		if (glVersion_ < 450)
		{
			Warning("GL_VERSION < 4.5");
			//return false;
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

			glDebugMessageCallbackARB(&DebugMessageCallback, NULL);
			
			glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
			glDebugMessageControlARB(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR_ARB, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			glDebugMessageControlARB(GL_DONT_CARE, GL_DEBUG_TYPE_PERFORMANCE_ARB, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			glDebugMessageControlARB(GL_DONT_CARE, GL_DEBUG_TYPE_PORTABILITY_ARB, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			glDebugMessageControlARB(GL_DONT_CARE, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			
			//glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
			//glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);

		}
#endif

		SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &window_.redBits);
		SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &window_.greenBits);
		SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &window_.blueBits);
		SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &window_.depthBits);
		SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &window_.alphaBits);
		SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &window_.stencilBits);


		window_.w = _w;
		window_.h = _h;

		GL_CHECK(glCreateVertexArrays(1, &shared_vertex_array_));
		GL_CHECK(glBindVertexArray(shared_vertex_array_));
		GL_CHECK(glEnable(GL_PROGRAM_POINT_SIZE));
		GL_CHECK(glEnable(GL_DEPTH_TEST));
		GL_CHECK(glDisable(GL_SCISSOR_TEST));
		GL_CHECK(glDepthMask(GL_TRUE));
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &gl_max_vertex_attribs_);


		ext_.ARB_vertex_attrib_binding = gl_extensions_.count("GL_ARB_vertex_attrib_binding");
		ext_.ARB_direct_state_access = gl_extensions_.count("GL_ARB_direct_state_access");
		ext_.EXT_direct_state_access = gl_extensions_.count("GL_EXT_direct_state_access");

		glGetIntegerv(GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT, &gl_texture_buffer_offset_alignment_);
		glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &gl_max_shader_storage_block_size_);
		glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &gl_max_uniform_block_size_);

		GLint NumFormats = 0; 
		glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &NumFormats); 
		compressedFormats_.resize(NumFormats);
		glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, compressedFormats_.data());


		SDL_GL_MakeCurrent(windowHandle_, NULL);

		return true;
	}

	void OpenGLRenderContext::process_command_list(const std::vector<RenderCommand>& cmds)
	{
		assert(windowHandle_);
		for (const auto& c : cmds)
		{
			std::visit(*this, c);
		}
	}

	void OpenGLRenderContext::destroy_window()
	{
		if (glcontext_)
			SDL_GL_DeleteContext(glcontext_);
		if (windowHandle_)
			SDL_DestroyWindow(windowHandle_);
	}

	void OpenGLRenderContext::start_rendering()
	{
		SDL_GL_MakeCurrent(windowHandle_, glcontext_);
		//glewInit();
	}

	void OpenGLRenderContext::stop_rendering()
	{
		SDL_GL_MakeCurrent(windowHandle_, NULL);
	}

	void OpenGLRenderContext::setup_textures(const TextureBinding* textures, size_t n)
	{
		for (auto j = 0; j < n; ++j)
		{
			if (textures[j].handle.isValid())
			{
				if (active_textures_[j] != textures[j].handle)
				{
					active_textures_[j] = textures[j].handle;
					const TextureData& tdata = texture_map_.at(textures[j].handle);
					if (gl_version_450_ || ext_.ARB_direct_state_access)
					{
						GL_CHECK(glBindTextureUnit(j, tdata.texture));
					}
					else if (ext_.EXT_direct_state_access)
					{
						GL_CHECK(glBindMultiTextureEXT(GL_TEXTURE0 + j, tdata.target, tdata.texture));
					}
					else
					{
						GL_CHECK(glActiveTexture(GL_TEXTURE0 + j));
						GL_CHECK(glBindTexture(tdata.target, tdata.texture));
					}
				}
			}
		}
	}

	void OpenGLRenderContext::setup_uniforms(ProgramData& program_data, const UniformMap& uniforms)
	{
		UniformBinder binder;
		for (const auto& cb : uniforms)
		{
			auto location = program_data.uniform_location_map.find(cb.first);
			GLint uniform_location;
			if (location != std::end(program_data.uniform_location_map))
			{
				uniform_location = location->second;
			}
			else {
				GL_CHECK(uniform_location = glGetUniformLocation(program_data.program, cb.first.c_str()));
				program_data.uniform_location_map.emplace(cb.first, uniform_location);
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

	void OpenGLRenderContext::compute(const RenderPass& pass)
	{
		for (int i = 0; i < pass.compute_items.size(); ++i)
		{
			auto* item = &pass.compute_items[i];
			if (!item->program.isValid())
			{
				if (item->fence.isValid())
				{
					GLsync sync = fence_map_.at(item->fence);
					if (item->wait_sync_client)
					{
						GLenum result{};
						GL_CHECK(result = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, item->wait_timeout));
						if (result == GL_TIMEOUT_EXPIRED)
						{
							Warning("glClientWaitSync timeout expired!");
						}
					}
					else
					{
						GL_CHECK(glWaitSync(sync, 0, GL_TIMEOUT_IGNORED));
					}
				}
				if (item->barrier_bits)
				{
					GLbitfield bits = MapBarrierBits(item->barrier_bits);
					GL_CHECK(glMemoryBarrier(bits));
				}
				continue;
			}

			ProgramData& program_data = program_map_.at(item->program);
			if (active_program_ != item->program)
			{
				assert(item->program.isValid());
				GL_CHECK(glUseProgram(program_data.program));
				active_program_ = item->program;
			}

			if ( ! item->uniforms.empty() ) {
				setup_uniforms(program_data, item->uniforms);
			}

			setup_textures(&item->textures[0], item->textures.size());

			assert(item->images.size() <= MAX_IMAGE_UNITS);
			for (uint k = 0; k < item->images.size(); ++k)
			{
				const auto& img = item->images[k];
				if (img.handle.isValid())
				{
					const auto& tdata = texture_map_.at(img.handle);
					if (active_imagetexes_[k] != img.handle)
					{
						active_imagetexes_[i] = img.handle;
						GL_CHECK(glBindImageTexture(k, tdata.texture, img.level, img.layered, img.layer, MapAccess(img.access), s_texture_format[static_cast<size_t>(img.format)].internal_format) );
					}
				}
			}

			GL_CHECK(glDispatchCompute(item->num_groups_x, item->num_groups_y, item->num_groups_z));

			if (item->fence.isValid())
			{
				GLsync syncObj;
				GL_CHECK(
					syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0)
				);
				fence_map_[item->fence] = syncObj;
			}
		}
	}

	bool OpenGLRenderContext::frame(const Frame* frame)
	{
		for (auto& pass : frame->render_passes)
		{
			if (!pass.compute_items.empty())
			{
				compute(pass);
			}

			if (pass.render_items.empty() || !pass.frame_buffer.isValid())
				continue;

			for (int i = 0; i < pass.constant_buffers.size(); ++i)
			{
				if (pass.constant_buffers[i].handle.isValid())
				{
					const auto& cbuf = pass.constant_buffers[i];
					if (cbuf.offset == 0 && cbuf.size == 0) {
						GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, i, constant_buffer_map_.at(cbuf.handle).buffer));
					} else {
						GL_CHECK(glBindBufferRange(GL_UNIFORM_BUFFER, i, constant_buffer_map_.at(cbuf.handle).buffer, cbuf.offset, cbuf.size));
					}
				}
			}

			ushort fb_width, fb_height;

			if (pass.frame_buffer.internal() > 0)
			{
				FrameBufferData& fb_data = frame_buffer_map_.at(pass.frame_buffer);
				fb_width = fb_data.width;
				fb_height = fb_data.height;
				if (active_fb_ != pass.frame_buffer)
				{
					GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fb_data.frame_buffer));
					active_fb_ = pass.frame_buffer;
				}
			}
			else 
			{
				fb_width = window_.w;
				fb_height = window_.h;
				if (active_fb_.internal() > 0)
				{
					GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
					active_fb_ = FrameBufferHandle{ 0 };
				}
			}

			(void)fb_width;
			(void)fb_height;

			GLbitfield clear_bits = 0;
			if (pass.clear_bits & GLS_CLEAR_COLOR) {
				clear_bits |= GL_COLOR_BUFFER_BIT;
			}
			if (pass.clear_bits & GLS_CLEAR_DEPTH) {
				clear_bits |= GL_DEPTH_BUFFER_BIT;
			}
			if (pass.clear_bits & GLS_CLEAR_STENCIL) {
				clear_bits |= GL_STENCIL_BUFFER_BIT;
			}

			if (clear_bits) {
				set_state(0, false);
				if (scissor_test_)
				{
					scissor_test_ = false;
					GL_CHECK(glDisable(GL_SCISSOR_TEST));
				}
				if (active_clear_color_ != pass.clear_color)
				{
					GL_CHECK(glClearColor(pass.clear_color.r, pass.clear_color.g, pass.clear_color.b, pass.clear_color.a));
					active_clear_color_ = pass.clear_color;
				}
				GL_CHECK(glClear(clear_bits));
			}

			for (uint i = 0; i < pass.render_items.size(); ++i)
			{
				auto* prev = i < 0 ? &pass.render_items[i - 1] : nullptr;
				auto* item = &pass.render_items[i];

				if (item->program == ProgramHandle::invalid)
				{
					continue;
				}

				set_state(item->state_bits, false);
				if (scissor_test_ != item->scissor)
				{
					if (item->scissor)	GL_CHECK(glEnable(GL_SCISSOR_TEST));
					else				GL_CHECK(glDisable(GL_SCISSOR_TEST));

					scissor_test_ = item->scissor;
				}
				if (item->scissor)
				{
					GL_CHECK(glScissor(item->scissor_x, item->scissor_y, item->scissor_w, item->scissor_h));
				}

				ProgramData& program_data = program_map_.at(item->program);
				if (active_program_ != item->program)
				{
					assert(item->program.isValid());
					GL_CHECK(glUseProgram(program_data.program));
					active_program_ = item->program;
				}

				if ( ! item->uniforms.empty() ) {
					setup_uniforms(program_data, item->uniforms);
				}

				setup_textures(item->textures.data(), item->textures.size());

				bool vb_change = false;

				
				for (uint j = 0; j < item->vbs.size(); ++j)
				{
					if (item->vbs[j] != active_vbs_[j])
					{
						vb_change = true;
						break;
					}
				}

				bool layout_change = false;
				if (vb_change)
				{
					if (gl_version_430_ || ext_.ARB_vertex_attrib_binding)
					{
						/* change layout if needed */
						if (!item->vertexDecl.empty() && active_vertex_layout_ != item->vertexDecl.handle())
						{
							active_vertex_decl_ = item->vertexDecl;
							active_vertex_layout_ = item->vertexDecl.handle();
							GLuint const vao = vertex_array_map_.at(item->vertexDecl.handle());
							GL_CHECK(glBindVertexArray(vao));
							layout_change = true;
						}

						for (uint j = 0; j < active_vertex_decl_.size(); ++j)
						{
							const auto& attr = active_vertex_decl_[j];
							if (active_vbs_[attr.binding] == item->vbs[attr.binding]) { continue; };

							const auto& binding = item->vbs[attr.binding];
							if (binding.handle.isValid())
							{
								const auto& vb_data = vertex_buffer_map_.at(binding.handle);
								GL_CHECK(glBindVertexBuffer(attr.binding, vb_data.buffer, binding.offset, attr.stride));
								if (layout_change)
								{
									GL_CHECK(glVertexBindingDivisor(attr.binding, attr.divisor));
								}
								active_vbs_[attr.binding] = item->vbs[attr.binding];
							}
						}
					}
					else
					{
						active_vertex_decl_ = item->vertexDecl;
						ushort active_binding = 0xffff;
						for (uint j = 0; j < active_vertex_decl_.size(); ++j)
						{
							const auto& attr = active_vertex_decl_[j];
							if (attr.binding != active_binding)
							{
								active_binding = attr.binding;
								const auto& binding = item->vbs[attr.binding];
								if (binding.handle.isValid())
								{
									const auto& vb_data = vertex_buffer_map_.at(binding.handle);
									GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vb_data.buffer));
									active_vbs_[attr.binding] = item->vbs[attr.binding];
								}
							}
							const GLenum type = MapAttribType(attr.type);
							if (attr.enabled) GL_CHECK(glEnableVertexAttribArray(j));
							else GL_CHECK(glDisableVertexAttribArray(j));
							GL_CHECK(glVertexAttribPointer(j, attr.count, type, (attr.normalized ? GL_TRUE : GL_FALSE), attr.stride, reinterpret_cast<void*>(attr.offset)));
							GL_CHECK(glVertexAttribDivisor(j, attr.divisor));
						}
						//GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
					}
				}

				// Element buffer setup
				if ((!prev || prev->ib != item->ib) && item->ib.isValid())
				{
					const auto& ib = index_buffer_map_.at(item->ib);
					GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.buffer));
					active_ib_type_ = ib.type == IndexBufferType::U16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
				}

				if (!item->vertexAttribs.empty())
				{
					VertexAttribSetter vas;
					for (const auto& e : item->vertexAttribs)
					{
						vas.update(e.first, e.second);
					}
				}

				const GLenum mode = MapDrawMode(item->primitive_type);
				const GLsizei count = item->vertex_count;
				if (item->ib.isValid())
				{
					const GLuint base_vertex = item->vb_offset;					
					
					GL_CHECK(glDrawElementsInstancedBaseVertex(
						mode,
						count,
						active_ib_type_,
						reinterpret_cast<void*>(static_cast<GLintptr>(item->ib_offset)),
						static_cast<GLsizei>(item->instance_count),
						base_vertex));
				}
				else
				{
					glDrawArraysInstanced(
						mode,
						item->vb_offset,
						static_cast<GLsizei>(count),
						static_cast<GLsizei>(item->instance_count));
				}

			} // render_items
		}

		SDL_GL_SwapWindow(windowHandle_);

		return true;
	}

	glm::ivec2 OpenGLRenderContext::get_window_size() const
	{
		return glm::ivec2(window_.w, window_.h);
	}

	OpenGLRenderContext::~OpenGLRenderContext()
	{
#if _DEBUG
		Info("OpenGLRenderContext::dtor");
#endif
		SDL_GL_MakeCurrent(windowHandle_, glcontext_);
		
		// freeing resources
		std::vector<GLuint> list1;
		std::vector<GLuint> list2;

		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		for (auto& r : frame_buffer_map_) {
			list1.push_back(r.second.frame_buffer);
			list2.push_back(r.second.depth_render_buffer);
		}
		GL_CHECK(glDeleteRenderbuffers(GLsizei(list2.size()), list2.data()));
		GL_CHECK(glDeleteFramebuffers(GLsizei(list1.size()), list1.data()));

		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
		GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));

		list1.clear();
		for (auto& r : texture_map_) {
			list1.push_back(r.second.texture);
		}
		GL_CHECK(glDeleteTextures(GLsizei(list1.size()), list1.data()));

		for (auto& r : shader_map_) {
			GL_CHECK(glDeleteShader(r.second.shader));
		}

		GL_CHECK(glUseProgram(0));

		for (auto& r : program_map_) {
			GL_CHECK(glDeleteProgram(r.second.program));
		}

		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
		GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
		GL_CHECK(glBindBuffer(GL_TEXTURE_BUFFER, 0));

		list1.clear();
		for (auto& r : vertex_buffer_map_) {
			list1.push_back(r.second.buffer);
		}
		for (auto& r : index_buffer_map_) {
			list1.push_back(r.second.buffer);
		}
		for (auto& r : constant_buffer_map_) {
			list1.push_back(r.second.buffer);
		}
		for (auto& r : texture_buffer_map_) {
			list1.push_back(r.second.buffer);
		}
		GL_CHECK(glDeleteBuffers(GLsizei(list1.size()), list1.data()));
		GL_CHECK(glDeleteVertexArrays(1, &shared_vertex_array_));

		list1.clear();
		for (auto& r : vertex_array_map_) {
			list1.push_back(r.second);
		}
		GL_CHECK(glDeleteVertexArrays(GLsizei(list1.size()), list1.data()));
		
		frame_buffer_map_.clear();
		vertex_buffer_map_.clear();
		index_buffer_map_.clear();
		texture_map_.clear();
		shader_map_.clear();
		program_map_.clear();
		vertex_array_map_.clear();

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

}
