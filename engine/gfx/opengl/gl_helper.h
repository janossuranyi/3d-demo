#pragma once
#include "gfx/opengl/gl_context.h"

namespace gfx {

	void GLAPIENTRY DebugMessageCallback(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar* message,
		const GLvoid* userParam);
		
	void CheckOpenGLError(const char* stmt, const char* fname, int line);
	GLenum MapAccess(Access a);
	GLenum MapDrawMode(PrimitiveType p);
	GLenum MapTextureShape(TextureShape shape);
	GLenum MapShaderStage(const ShaderStage type);
	GLenum MapBufferUsage(const BufferUsage usage);
	GLenum MapAttribType(const AttributeType type);
	const char* MapShaderStageTitle(ShaderStage type);
	GLbitfield MapBarrierBits(uint32_t bits);

}