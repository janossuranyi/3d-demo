#include "gl_helper.h"
#include "logger.h"

namespace gfx {

	void CheckOpenGLError(const char* stmt, const char* fname, int line)
	{
		GLenum err = glGetError();
		if (err != GL_NO_ERROR)
		{
			Error("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
			abort();
		}
	}

	void GLAPIENTRY DebugMessageCallback(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar* message,
		const GLvoid* userParam);


	GLbitfield MapBarrierBits(uint32_t bits)
	{
		uint32_t result = 0;

		if (bits & barrier::AtomicCounter)
			result |= GL_ATOMIC_COUNTER_BARRIER_BIT;
		if (bits & barrier::BufferUpdate)
			result |= GL_BUFFER_UPDATE_BARRIER_BIT;
		if (bits & barrier::ClientMappedBuffer)
			result |= GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT;
		if (bits & barrier::FrameBuffer)
			result |= GL_FRAMEBUFFER_BARRIER_BIT;
		if (bits & barrier::QueryBuffer)
			result |= GL_QUERY_BUFFER_BARRIER_BIT;
		if (bits & barrier::ShaderImageAccess)
			result |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
		if (bits & barrier::TextureUpdate)
			result |= GL_TEXTURE_UPDATE_BARRIER_BIT;
		if (bits & barrier::TransformFeedback)
			result |= GL_TRANSFORM_FEEDBACK_BARRIER_BIT;
		if (bits & barrier::Uniform)
			result |= GL_UNIFORM_BARRIER_BIT;

		return result;
	}

	const char* MapShaderStageTitle(ShaderStage type)
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


	GLenum MapAttribType(const AttributeType type)
	{
		switch (type) {
		case AttributeType::Byte:
			return GL_BYTE;
		case AttributeType::UByte:
			return GL_UNSIGNED_BYTE;
		case AttributeType::Short:
			return GL_SHORT;
		case AttributeType::UShort:
			return GL_UNSIGNED_SHORT;
		case AttributeType::Int:
			return GL_INT;
		case AttributeType::UInt:
			return GL_UNSIGNED_INT;
		case AttributeType::Half:
			return GL_HALF_FLOAT;
		default:
			return GL_FLOAT;
		}

	}

	GLenum MapBufferUsage(const BufferUsage usage)
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

	GLenum MapShaderStage(const ShaderStage type)
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

	GLenum MapTextureShape(TextureShape shape)
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

	GLenum MapDrawMode(PrimitiveType p)
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

	GLenum MapAccess(Access a)
	{
		switch (a)
		{
		case Access::Read:		return GL_READ_ONLY;
		case Access::Write:		return GL_WRITE_ONLY;
		case Access::ReadWrite:	return GL_READ_WRITE;
		}
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