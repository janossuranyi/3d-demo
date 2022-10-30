#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <cassert>
#include <unordered_map>
#include <array>
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

	static inline GLenum MapAttribType(const AttributeType type)
	{
		switch (type) {
		case AttributeType::Byte:
			return GL_BYTE;
		case AttributeType::UnsignedByte:
			return GL_UNSIGNED_BYTE;
		case AttributeType::Short:
			return GL_SHORT;
		case AttributeType::UnsignedShort:
			return GL_UNSIGNED_SHORT;
		case AttributeType::Int:
			return GL_INT;
		case AttributeType::UnsignedInt:
			return GL_UNSIGNED_INT;
		case AttributeType::Half:
			return GL_HALF_FLOAT;
		default:
			return GL_FLOAT;
		}

	}

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

	static GLenum MapTextureShape(TextureShape shape)
	{
		switch (shape) {
		case TextureShape::D1:
			return GL_TEXTURE_1D;
		case TextureShape::D2:
			return GL_TEXTURE_2D;
		case TextureShape::D3:
			return GL_TEXTURE_3D;
		case TextureShape::CubeMap:
			return GL_TEXTURE_CUBE_MAP;
		}
	}

	static GLenum MapDrawMode(PrimitiveType p)
	{
		switch (p)
		{
		case PrimitiveType::Lines:
			return GL_LINES;
		case PrimitiveType::LineLoop:
			return GL_LINE_LOOP;
		case PrimitiveType::LineStrip:
			return GL_LINE_STRIP;
		case PrimitiveType::Point:
			return GL_POINTS;
		case PrimitiveType::Triangles:
			return GL_TRIANGLES;
		case PrimitiveType::TriangleFan:
			return GL_TRIANGLE_FAN;
		case PrimitiveType::TriangleStrip:
			return GL_TRIANGLE_STRIP;
		}
	}

	static GLenum MapAccess(Access a)
	{
		switch (a)
		{
		case Access::Read:		return GL_READ_ONLY;
		case Access::Write:		return GL_WRITE_ONLY;
		case Access::ReadWrite:	return GL_READ_WRITE;
		}
	}

	struct VertexAttribFormat {
		GLint size;
		GLenum type;
		GLboolean normalized;
		GLsizeiptr pointer;
	};

	struct VertexLayout {
		std::vector<VertexAttribFormat> attributes;
		uint32_t stride;
	};

	static const VertexLayout drawVertLayout = {
		{
			{4,		GL_FLOAT,			GL_FALSE,	0},
			{2,		GL_HALF_FLOAT,		GL_FALSE,	16},
			{4,		GL_UNSIGNED_BYTE,	GL_TRUE,	20},
			{4,		GL_UNSIGNED_BYTE,	GL_TRUE,	24},
			{4,		GL_UNSIGNED_BYTE,	GL_TRUE,	28}
		}, 32
	};

	static const VertexLayout shadowVertLayout = { 
		{
			{4,		GL_FLOAT,			GL_FALSE,	0},
			{2,		GL_HALF_FLOAT,		GL_FALSE,	16},
		}, 20
	};

	static const std::array<VertexLayout, 2> s_vertexLayouts = { drawVertLayout, shadowVertLayout };

	// GL TextureFormatInfo.
	struct TextureFormatInfo {
		GLenum internal_format;
		GLenum internal_format_srgb;
		GLenum format;
		GLenum type;
		bool supported;
	};

	TextureFormatInfo s_texture_format[] = {
		{GL_ALPHA,              GL_ZERO,         GL_ALPHA,            GL_UNSIGNED_BYTE,                false}, // A8
		{GL_R8,                 GL_ZERO,         GL_RED,              GL_UNSIGNED_BYTE,                false}, // R8
		{GL_R8I,                GL_ZERO,         GL_RED,              GL_BYTE,                         false}, // R8I
		{GL_R8UI,               GL_ZERO,         GL_RED,              GL_UNSIGNED_BYTE,                false}, // R8U
		{GL_R8_SNORM,           GL_ZERO,         GL_RED,              GL_BYTE,                         false}, // R8S
		{GL_R16,                GL_ZERO,         GL_RED,              GL_UNSIGNED_SHORT,               false}, // R16
		{GL_R16I,               GL_ZERO,         GL_RED,              GL_SHORT,                        false}, // R16I
		{GL_R16UI,              GL_ZERO,         GL_RED,              GL_UNSIGNED_SHORT,               false}, // R16U
		{GL_R16F,               GL_ZERO,         GL_RED,              GL_HALF_FLOAT,                   false}, // R16F
		{GL_R16_SNORM,          GL_ZERO,         GL_RED,              GL_SHORT,                        false}, // R16S
		{GL_R32I,               GL_ZERO,         GL_RED,              GL_INT,                          false}, // R32I
		{GL_R32UI,              GL_ZERO,         GL_RED,              GL_UNSIGNED_INT,                 false}, // R32U
		{GL_R32F,               GL_ZERO,         GL_RED,              GL_FLOAT,                        false}, // R32F
		{GL_RG8,                GL_ZERO,         GL_RG,               GL_UNSIGNED_BYTE,                false}, // RG8
		{GL_RG8I,               GL_ZERO,         GL_RG,               GL_BYTE,                         false}, // RG8I
		{GL_RG8UI,              GL_ZERO,         GL_RG,               GL_UNSIGNED_BYTE,                false}, // RG8U
		{GL_RG8_SNORM,          GL_ZERO,         GL_RG,               GL_BYTE,                         false}, // RG8S
		{GL_RG16,               GL_ZERO,         GL_RG,               GL_UNSIGNED_SHORT,               false}, // RG16
		{GL_RG16I,              GL_ZERO,         GL_RG,               GL_SHORT,                        false}, // RG16I
		{GL_RG16UI,             GL_ZERO,         GL_RG,               GL_UNSIGNED_SHORT,               false}, // RG16U
		{GL_RG16F,              GL_ZERO,         GL_RG,               GL_FLOAT,                        false}, // RG16F
		{GL_RG16_SNORM,         GL_ZERO,         GL_RG,               GL_SHORT,                        false}, // RG16S
		{GL_RG32I,              GL_ZERO,         GL_RG,               GL_INT,                          false}, // RG32I
		{GL_RG32UI,             GL_ZERO,         GL_RG,               GL_UNSIGNED_INT,                 false}, // RG32U
		{GL_RG32F,              GL_ZERO,         GL_RG,               GL_FLOAT,                        false}, // RG32F
		{GL_RGB8,               GL_SRGB8,        GL_RGB,              GL_UNSIGNED_BYTE,                false}, // RGB8
		{GL_RGB8I,              GL_ZERO,         GL_RGB,              GL_BYTE,                         false}, // RGB8I
		{GL_RGB8UI,             GL_ZERO,         GL_RGB,              GL_UNSIGNED_BYTE,                false}, // RGB8U
		{GL_RGB8_SNORM,         GL_ZERO,         GL_RGB,              GL_BYTE,                         false}, // RGB8S
		{GL_RGBA8,              GL_SRGB8_ALPHA8, GL_BGRA,             GL_UNSIGNED_BYTE,                false}, // BGRA8
		{GL_RGBA8,              GL_SRGB8_ALPHA8, GL_RGBA,             GL_UNSIGNED_BYTE,                false}, // RGBA8
		{GL_RGBA8I,             GL_ZERO,         GL_RGBA,             GL_BYTE,                         false}, // RGBA8I
		{GL_RGBA8UI,            GL_ZERO,         GL_RGBA,             GL_UNSIGNED_BYTE,                false}, // RGBA8U
		{GL_RGBA8_SNORM,        GL_ZERO,         GL_RGBA,             GL_BYTE,                         false}, // RGBA8S
		{GL_RGBA16,             GL_ZERO,         GL_RGBA,             GL_UNSIGNED_SHORT,               false}, // RGBA16
		{GL_RGBA16I,            GL_ZERO,         GL_RGBA,             GL_SHORT,                        false}, // RGBA16I
		{GL_RGBA16UI,           GL_ZERO,         GL_RGBA,             GL_UNSIGNED_SHORT,               false}, // RGBA16U
		{GL_RGBA16F,            GL_ZERO,         GL_RGBA,             GL_HALF_FLOAT,                   false}, // RGBA16F
		{GL_RGBA16_SNORM,       GL_ZERO,         GL_RGBA,             GL_SHORT,                        false}, // RGBA16S
		{GL_RGBA32I,            GL_ZERO,         GL_RGBA,             GL_INT,                          false}, // RGBA32I
		{GL_RGBA32UI,           GL_ZERO,         GL_RGBA,             GL_UNSIGNED_INT,                 false}, // RGBA32U
		{GL_RGBA32F,            GL_ZERO,         GL_RGBA,             GL_FLOAT,                        false}, // RGBA32F
		{GL_RGBA4,              GL_ZERO,         GL_RGBA,             GL_UNSIGNED_SHORT_4_4_4_4_REV,   false}, // RGBA4
		{GL_RGB5_A1,            GL_ZERO,         GL_RGBA,             GL_UNSIGNED_SHORT_1_5_5_5_REV,   false}, // RGB5A1
		{GL_RGB10_A2,           GL_ZERO,         GL_RGBA,             GL_UNSIGNED_INT_2_10_10_10_REV,  false}, // RGB10A2
		{GL_R11F_G11F_B10F,     GL_ZERO,         GL_RGB,              GL_UNSIGNED_INT_10F_11F_11F_REV, false}, // RG11B10F
		{GL_COMPRESSED_RGB,     GL_COMPRESSED_SRGB,GL_RGB,            GL_UNSIGNED_BYTE,                false}, // RGB8_COMPRESSED
		{GL_COMPRESSED_RGBA,    GL_COMPRESSED_SRGB_ALPHA,GL_RGBA,     GL_UNSIGNED_BYTE,                false}, // RGBA8_COMPRESSED
		{GL_DEPTH_COMPONENT16,  GL_ZERO,         GL_DEPTH_COMPONENT,  GL_UNSIGNED_SHORT,               false}, // D16
		{GL_DEPTH_COMPONENT24,  GL_ZERO,         GL_DEPTH_COMPONENT,  GL_UNSIGNED_INT,                 false}, // D24
		{GL_DEPTH24_STENCIL8,   GL_ZERO,         GL_DEPTH_STENCIL,    GL_UNSIGNED_INT_24_8,            false}, // D24S8
		{GL_DEPTH_COMPONENT32,  GL_ZERO,         GL_DEPTH_COMPONENT,  GL_UNSIGNED_INT,                 false}, // D32
		{GL_DEPTH_COMPONENT32F, GL_ZERO,         GL_DEPTH_COMPONENT,  GL_FLOAT,                        false}, // D16F
		{GL_DEPTH_COMPONENT32F, GL_ZERO,         GL_DEPTH_COMPONENT,  GL_FLOAT,                        false}, // D24F
		{GL_DEPTH_COMPONENT32F, GL_ZERO,         GL_DEPTH_COMPONENT,  GL_FLOAT,                        false}, // D32F
		{GL_STENCIL_INDEX8,     GL_ZERO,         GL_STENCIL_INDEX,    GL_UNSIGNED_BYTE,                false}, // D0S8
	};

	static const std::unordered_map<BlendEquation, GLenum> s_blend_equation_map = {
		{BlendEquation::Add, GL_FUNC_ADD},
		{BlendEquation::Subtract, GL_FUNC_SUBTRACT},
		{BlendEquation::ReverseSubtract, GL_FUNC_REVERSE_SUBTRACT},
		{BlendEquation::Min, GL_MIN},
		{BlendEquation::Max, GL_MAX}
	};
	static const std::unordered_map<BlendFunc, GLenum> s_blend_func_map = {
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
	static const std::unordered_map<TextureWrap, GLenum> s_texture_wrap_map = {
		{TextureWrap::ClampToBorder, GL_CLAMP_TO_BORDER},
		{TextureWrap::ClampToEdge, GL_CLAMP_TO_EDGE},
		{TextureWrap::MirroredRepeat, GL_MIRRORED_REPEAT},
		{TextureWrap::Repeat, GL_REPEAT}
	};
	static const std::unordered_map<TextureFilter, GLenum> s_texture_filter_map = {
		{TextureFilter::Linear, GL_LINEAR},
		{TextureFilter::LinearNearest, GL_LINEAR_MIPMAP_NEAREST},
		{TextureFilter::Nearest, GL_NEAREST},
		{TextureFilter::NearestLinear, GL_NEAREST_MIPMAP_LINEAR},
		{TextureFilter::LinearLinear, GL_LINEAR_MIPMAP_LINEAR}
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
		void operator()(const std::vector<float>& value) {
			glUniform1fv(uniform_location_, value.size(), (const GLfloat*)value.data());
		}
		void operator()(const std::vector<glm::vec4>& value) {
			glUniform4fv(uniform_location_, value.size(), (const GLfloat*)value.data());
		}
		void update(GLint location, const UniformData& value) {
			uniform_location_ = location;
			std::visit(*this, value);
		}
	private:
		GLint uniform_location_;
	};

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

	void OpenGLRenderContext::operator()(const cmd::CreateConstantBuffer& cmd)
	{
		if (constant_buffer_map_.count(cmd.handle) > 0)
			return;

		GLuint buffer = static_cast<GLuint>(0xffff);
		const GLenum _usage = MapBufferUsage(cmd.usage);

		GL_CHECK(glGenBuffers(1, &buffer));
		assert(buffer != 0xffff);

		const uint32_t _size = cmd.data.data() ? cmd.data.size() : cmd.size;

		GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, buffer));
		if (cmd.data.data())
			GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, GLsizeiptr(cmd.data.size()), cmd.data.data(), _usage));
		else
			GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, GLsizeiptr(cmd.size), nullptr, _usage));

		constant_buffer_map_.emplace(cmd.handle, ConstantBufferData{ buffer, _size, cmd.usage });
	}

	void OpenGLRenderContext::operator()(const cmd::UpdateConstantBuffer& cmd)
	{
		if (cmd.data.data() == nullptr)
			return;

		const auto result = constant_buffer_map_.find(cmd.handle);
		if (result == constant_buffer_map_.end())
			return;

		const auto& data = result->second;
		const GLsizeiptr size = cmd.size > 0 ? GLsizeiptr(cmd.size) : cmd.data.size();
		GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, data.buffer));
		GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, GLintptr(cmd.offset), size, cmd.data.data()));
	}

	void OpenGLRenderContext::operator()(const cmd::DeleteConstantBuffer& cmd)
	{
		const auto result = constant_buffer_map_.find(cmd.handle);
		if (result == constant_buffer_map_.end())
			return;

		const auto& data = result->second;

		GL_CHECK(glDeleteBuffers(1, &data.buffer));
		constant_buffer_map_.erase(cmd.handle);
#ifdef _DEBUG
		Info("Uniform buffer %d deleted", cmd.handle);
#endif

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

#ifdef _DEBUG
		Info("Vertex buffer %d deleted", cmd.handle);
#endif
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
		const auto& data = result->second;
		const GLsizeiptr size = data.size > 0 ? GLsizeiptr(data.size) : cmd.data.size();
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,		data.buffer));
		GL_CHECK(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,	GLintptr(cmd.offset), size, cmd.data.data()));

	}

	void OpenGLRenderContext::operator()(const cmd::DeleteIndexBuffer& cmd)
	{
		const auto result = index_buffer_map_.find(cmd.handle);
		GL_CHECK(glDeleteBuffers(1, &result->second.buffer));
		index_buffer_map_.erase(cmd.handle);

#ifdef _DEBUG
		Info("Uniform buffer %d deleted", cmd.handle);
#endif

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
		GL_CHECK(glDeleteShader(result->second.shader));
		shader_map_.erase(cmd.handle);

#ifdef _DEBUG
		Info("Shader %d deleted", cmd.handle);
#endif

	}

	void OpenGLRenderContext::operator()(const cmd::CreateProgram& cmd)
	{
		GLuint program;
		GL_CHECK(program = glCreateProgram());
		if (program)
		{
			const ProgramData data = { program, false, {}};
			program_map_.emplace(cmd.handle, data);
		}
	}

	void OpenGLRenderContext::operator()(const cmd::LinkProgram& cmd)
	{
		auto& p_data = program_map_.at(cmd.handle);
		for (auto& s_handle : cmd.shaders)
		{
			auto& s_data = shader_map_.at(s_handle);
			GL_CHECK(glAttachShader(p_data.program, s_data.shader));
		}
		GL_CHECK(glLinkProgram(p_data.program));
		GLint result = GL_FALSE;

		GL_CHECK(glGetProgramiv(p_data.program, GL_LINK_STATUS, &result));

		if (result == GL_FALSE)
		{
			GLint infologLen;
			GL_CHECK(glGetProgramiv(p_data.program, GL_INFO_LOG_LENGTH, &infologLen));
			if (infologLen > 0) {
				std::vector<char> logBuf(infologLen);
				GL_CHECK(glGetProgramInfoLog(p_data.program, infologLen, nullptr, logBuf.data()));
				Error("Linking of shader program failed: %s", logBuf.data());
			}
			return;
		}
		p_data.linked = true;
	}

	void OpenGLRenderContext::operator()(const cmd::DeleteProgram& cmd)
	{
		auto& p_data = program_map_.at(cmd.handle);
		GL_CHECK(glDeleteProgram(p_data.program));
		program_map_.erase(cmd.handle);

#ifdef _DEBUG
		Info("Program %d deleted", cmd.handle);
#endif

	}

	void OpenGLRenderContext::operator()(const cmd::CreateTexture1D& cmd) {}

	void OpenGLRenderContext::operator()(const cmd::DeleteTexture& cmd) 
	{
		auto& t_data = texture_map_.at(cmd.handle);
		GL_CHECK(glDeleteTextures(1, &t_data.texture));
		texture_map_.erase(cmd.handle);

#ifdef _DEBUG
		Info("Texture %d deleted", cmd.handle);
#endif

	}

	void OpenGLRenderContext::operator()(const cmd::CreateTexture2D& cmd)
	{
		GLuint texture;
		const GLenum target = GL_TEXTURE_2D;
		GL_CHECK(glGenTextures(1, &texture));
		GL_CHECK(glBindTexture(target, texture));
		auto& texinfo = s_texture_format[static_cast<int>( cmd.format )];

		GL_CHECK(glTexImage2D(target, 0, 
			(cmd.srgb ? texinfo.internal_format_srgb : texinfo.internal_format), 
			cmd.width, 
			cmd.height, 
			0, 
			texinfo.format, 
			texinfo.type, 
			cmd.data.data()));

		const GLenum wrap = s_texture_wrap_map.at(cmd.wrap);
		const GLenum min_filter = s_texture_filter_map.at(cmd.min_filter);
		const GLenum mag_filter = s_texture_filter_map.at(cmd.mag_filter);

		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap));

		if (cmd.mipmap)
		{
			GL_CHECK(glGenerateMipmap(target));
		}
		GL_CHECK(glBindTexture(target, 0));

		const TextureData t_data{ texture, target, cmd.format };
		texture_map_.emplace(cmd.handle, t_data);
	}

	void OpenGLRenderContext::operator()(const cmd::CreateTextureCubeMap& cmd)
	{
		GLuint texture;
		const GLenum target = GL_TEXTURE_CUBE_MAP;
		GL_CHECK(glGenTextures(1, &texture));
		GL_CHECK(glBindTexture(target, texture));
		auto& texinfo = s_texture_format[static_cast<int>(cmd.format)];
		for (unsigned int face = 0; face < 6; ++face) {
			GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
				(cmd.srgb ? texinfo.internal_format_srgb : texinfo.internal_format),
				cmd.width,
				cmd.height,
				0,
				texinfo.format,
				texinfo.type,
				cmd.data[face].data()));
		}
		const GLenum wrap = s_texture_wrap_map.at(cmd.wrap);
		const GLenum min_filter = s_texture_filter_map.at(cmd.min_filter);
		const GLenum mag_filter = s_texture_filter_map.at(cmd.mag_filter);

		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap));
		GL_CHECK(glGenerateMipmap(target));
		GL_CHECK(glBindTexture(target, 0));

		const TextureData t_data{ texture, target };
		texture_map_.emplace(cmd.handle, t_data);
	}

	void OpenGLRenderContext::operator()(const cmd::CreateFramebuffer& cmd)
	{
		if (frame_buffer_map_.count(cmd.handle) > 0)
			return;

		FrameBufferData fb_data;

		fb_data.width = cmd.width;
		fb_data.height = cmd.height;

		GL_CHECK(glGenFramebuffers(1, &fb_data.frame_buffer));
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fb_data.frame_buffer));
		std::vector<GLenum> draw_buffers;
		int attachment{ 0 };
		for (auto& fb_texture : cmd.textures)
		{
			const auto& gl_texture = texture_map_.find(fb_texture);
			assert(gl_texture != texture_map_.end());

			draw_buffers.emplace_back(GL_COLOR_ATTACHMENT0 + attachment++);
			GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, draw_buffers.back(), GL_TEXTURE_2D, gl_texture->second.texture, 0));
		}
		GL_CHECK(glDrawBuffers(static_cast<GLsizei>(draw_buffers.size()), draw_buffers.data()));

		if (cmd.depth_stencil_texture.isValid())
		{
			auto& depth = texture_map_.at(cmd.depth_stencil_texture);
			// Stencil only
			if (depth.format == TextureFormat::D0S8)
			{
				GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth.texture, 0));
			}
			// depth only
			else if (
				depth.format == TextureFormat::D16 ||
				depth.format == TextureFormat::D16F ||
				depth.format == TextureFormat::D24 ||
				depth.format == TextureFormat::D24F ||
				depth.format == TextureFormat::D32 ||
				depth.format == TextureFormat::D32F)
			{
				GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth.texture, 0));
			}
			else // depth+stencil
			{
				GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth.texture, 0));
			}
		}
		else
		{
			// Create depth buffer.
			GL_CHECK(glGenRenderbuffers(1, &fb_data.depth_render_buffer));
			GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, fb_data.depth_render_buffer));
			GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, cmd.width, cmd.height));
			GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
				fb_data.depth_render_buffer));
		}

		// Check frame buffer status.
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			Error("[CreateFrameBuffer] The framebuffer is not complete. Status: 0x%x", status);
		}

		// Unbind.
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		// Add to map.
		frame_buffer_map_.emplace(cmd.handle, fb_data);
	}

	void OpenGLRenderContext::operator()(const cmd::DeleteFramebuffer& cmd)
	{
		auto& fbData = frame_buffer_map_.at(cmd.handle);
		GL_CHECK(glDeleteFramebuffers(1, &fbData.frame_buffer));
		GL_CHECK(glDeleteRenderbuffers(1, &fbData.depth_render_buffer));
#ifdef _DEBUG
		Info("Framebuffer %d deleted", cmd.handle);
#endif
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

		GL_CHECK(glCreateVertexArrays(1, &shared_vertex_array_));
		GL_CHECK(glBindVertexArray(shared_vertex_array_));
		GL_CHECK(glEnable(GL_PROGRAM_POINT_SIZE));
		GL_CHECK(glEnable(GL_DEPTH_TEST));
		GL_CHECK(glDisable(GL_SCISSOR_TEST));
		GL_CHECK(glDepthMask(GL_TRUE));
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &gl_max_vertex_attribs_);

		SDL_GL_MakeCurrent(windowHandle_, NULL);

		return true;
	}

	void OpenGLRenderContext::process_command_list(const std::vector<RenderCommand>& cmds)
	{
		assert(windowHandle_);
		for (auto& c : cmds)
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
	}

	void OpenGLRenderContext::stop_rendering()
	{
		SDL_GL_MakeCurrent(windowHandle_, NULL);
	}

	void OpenGLRenderContext::setup_textures(const TextureBindings& textures)
	{
		for (int j = 0; j < textures.size(); ++j) {
			if (textures[j].handle.isValid())
			{
				GL_CHECK(
					glActiveTexture(GL_TEXTURE0 + j)
				);

				const TextureData& tdata = texture_map_.at(textures[j].handle);
				GL_CHECK(
					glBindTexture(tdata.target, tdata.texture)
				);
			}
		}
	}

	void OpenGLRenderContext::setup_uniforms(ProgramData& program_data, const UniformMap& uniforms)
	{
		UniformBinder binder;
		for (auto& cb : uniforms)
		{
			auto location = program_data.uniform_location_map.find(cb.first);
			GLint uniform_location;
			if (location != program_data.uniform_location_map.end())
			{
				uniform_location = location->second;
			}
			else {
				GL_CHECK(uniform_location = glGetUniformLocation(program_data.program, cb.first.c_str()));
				program_data.uniform_location_map.emplace(cb.first, uniform_location);

				if (uniform_location == -1) {
					Warning("Uniform variable %s not found!!!", cb.first.c_str());
				}
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
						GL_CHECK(
							result = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, item->wait_timeout)
						);
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
				continue;
			}

			ProgramData& program_data = program_map_.at(item->program);
			if (active_program_ != item->program)
			{
				assert(item->program.isValid());
				GL_CHECK(glUseProgram(program_data.program));
				active_program_ = item->program;
			}

			setup_uniforms(program_data, item->uniforms);
			setup_textures(item->textures);

			for (int k = 0; k < item->images.size(); ++k)
			{
				auto& img = item->images[k];
				if (img.handle.isValid())
				{
					const TextureData& tdata = texture_map_.at(img.handle);
					GL_CHECK(
						glBindImageTexture(k, tdata.texture, img.level, img.layered, img.layer, MapAccess(img.access), s_texture_format[static_cast<size_t>(img.format)].internal_format)
					);
				}
			}

			GL_CHECK(
				glDispatchCompute(item->num_groups_x, item->num_groups_y, item->num_groups_z)
			);

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
				if (pass.constant_buffers[i].isValid())
				{
					GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, i, constant_buffer_map_.at(pass.constant_buffers[i]).buffer));
				}
			}

			uint16_t fb_width, fb_height;

			if (pass.frame_buffer.internal() > 0) {
				FrameBufferData& fb_data = frame_buffer_map_.at(pass.frame_buffer);
				fb_width = fb_data.width;
				fb_height = fb_data.height;
				if (active_fb_ != pass.frame_buffer)
				{
					GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fb_data.frame_buffer));
					active_fb_ = pass.frame_buffer;
				}
			}
			else {
				fb_width = window_.w;
				fb_height = window_.h;
				if (active_fb_.internal() > 0)
				{
					GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
					active_fb_ = FrameBufferHandle(0);
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
				scissor_test_ = false;
				GL_CHECK(glDisable(GL_SCISSOR_TEST));
				GL_CHECK(glClearColor(pass.clear_color.r, pass.clear_color.g, pass.clear_color.b, pass.clear_color.a));
				GL_CHECK(glClear(clear_bits));
			}

			for (auto i = 0; i < pass.render_items.size(); ++i)
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

				setup_uniforms(program_data, item->uniforms);
				setup_textures(item->textures);

				if (active_vb_ != item->vb)
				{
					auto& vb_data = vertex_buffer_map_.at(item->vb);
					GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vb_data.buffer));
				}
				if ((!prev || prev->ib != item->ib) && item->ib.isValid())
				{
					auto& ib = index_buffer_map_.at(item->ib);
					GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.buffer));
					active_ib_type_ = ib.type == IndexBufferType::U16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
				}

				if (active_vertex_decl_.empty() || item->vb != active_vb_)
				{
					if (item->vertexDecl.empty() == false)
					{
						active_vertex_decl_ = item->vertexDecl;
					}

					for (int j = 0; j < active_vertex_decl_.attributes().size(); ++j)
					{
						GL_CHECK(glDisableVertexAttribArray(j));
					}
					for (int j = 0; j < active_vertex_decl_.attributes().size(); ++j)
					{
						const VertexAttribute& attr = active_vertex_decl_.attributes()[j];
						GL_CHECK(glEnableVertexAttribArray(j));
						GL_CHECK(glVertexAttribPointer(
							j,
							attr.count,
							MapAttribType(attr.type),
							attr.normalized ? GL_TRUE : GL_FALSE,
							active_vertex_decl_.stride(),
							attr.offset));
					}
					active_vb_ = item->vb;
				}

				const GLenum mode = MapDrawMode(item->primitive_type);
				const GLsizei count = item->vertex_count;
				if (item->ib.isValid())
				{
					const int base_vertex = item->vb_offset; // s_vertexLayouts[static_cast<size_t>(active_vertex_decl_)].stride;
					GL_CHECK(glDrawElementsBaseVertex(
						mode,
						count,
						active_ib_type_,
						reinterpret_cast<void*>(static_cast<GLintptr>(item->ib_offset)),
						base_vertex));
				}
				else
				{
					glDrawArrays(
						mode,
						item->vb_offset, // s_vertexLayouts[static_cast<size_t>(active_vertex_decl_)].stride,
						count);
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
		GL_CHECK(glDeleteBuffers(GLsizei(list1.size()), list1.data()));
		GL_CHECK(glDeleteVertexArrays(1, &shared_vertex_array_));

		frame_buffer_map_.clear();
		vertex_buffer_map_.clear();
		index_buffer_map_.clear();
		texture_map_.clear();
		shader_map_.clear();
		program_map_.clear();

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
