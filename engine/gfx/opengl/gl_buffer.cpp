#include "gfx/opengl/gl_context.h"
#include "gfx/opengl/gl_helper.h"

#include "logger.h"

namespace gfx {

	void OpenGLRenderContext::operator()(const cmd::CreateVertexBuffer& cmd)
	{
		if (vertex_buffer_map_.count(cmd.handle) > 0)
			return;

		GLuint buffer = static_cast<GLuint>(0xffff);
		const GLenum _usage = MapBufferUsage(cmd.usage);

		GL_CHECK(glGenBuffers(1, &buffer));
		assert(buffer != 0xffff);

		const uint32_t _size = cmd.data.data() ? cmd.data.size() : cmd.size;

		GL_CHECK(
			glBindBuffer(GL_ARRAY_BUFFER, buffer));
		if (cmd.data.data())
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(cmd.data.size()), cmd.data.data(), _usage));
		else
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(cmd.size), nullptr, _usage));

		vertex_buffer_map_.emplace(cmd.handle, VertexBufferData{ buffer, _size, cmd.usage });
	}

	void OpenGLRenderContext::operator()(const cmd::CreateTextureBuffer& cmd)
	{
		if (texture_buffer_map_.count(cmd.handle) > 0)
			return;

		GLuint buffer = static_cast<GLuint>(0xffff);
		const GLenum _usage = MapBufferUsage(cmd.usage);

		GL_CHECK(glGenBuffers(1, &buffer));

		assert(buffer != 0xffff);

		const uint _size = cmd.data.data() ? cmd.data.size() : cmd.size;

		GL_CHECK(
			glBindBuffer(GL_TEXTURE_BUFFER, buffer));
		if (cmd.data.data())
			GL_CHECK(glBufferData(GL_TEXTURE_BUFFER, GLsizeiptr(cmd.data.size()), cmd.data.data(), _usage));
		else
			GL_CHECK(glBufferData(GL_TEXTURE_BUFFER, GLsizeiptr(cmd.size), nullptr, _usage));

		texture_buffer_map_.emplace(cmd.handle, TextureBufferData{ buffer, _size, cmd.usage });
	}

	void OpenGLRenderContext::operator()(const cmd::UpdateTextureBuffer& cmd)
	{
		if (cmd.data.data() == nullptr)
			return;

		const auto result = texture_buffer_map_.find(cmd.handle);
		if (result == texture_buffer_map_.end())
			return;

		const auto& data = result->second;
		const GLsizeiptr size = cmd.size > 0 ? GLsizeiptr(cmd.size) : cmd.data.size();
		GL_CHECK(glBindBuffer(GL_TEXTURE_BUFFER, data.buffer));
		GL_CHECK(glBufferSubData(GL_TEXTURE_BUFFER, GLintptr(cmd.offset), size, cmd.data.data()));
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

		char* mem{};
		if (gl_version_450_)
		{
			mem = reinterpret_cast<char*>(glMapNamedBuffer(data.buffer, GL_WRITE_ONLY)); GLC();
			if (mem) {
				std::memcpy(mem + cmd.offset, cmd.data.data(), size);
				GL_CHECK(glUnmapNamedBuffer(data.buffer));
			}
		}
		else
		{
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, data.buffer));
			mem = reinterpret_cast<char*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY)); GLC();
			if (mem) {
				std::memcpy(mem + cmd.offset, cmd.data.data(), size);
				GL_CHECK(glUnmapBuffer(GL_ARRAY_BUFFER));
			}
		}
		//GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, GLintptr(cmd.offset), size, cmd.data.data()));
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

	void OpenGLRenderContext::operator()(const cmd::DeleteTextureBuffer& cmd)
	{
		const auto result = texture_buffer_map_.find(cmd.handle);
		if (result == texture_buffer_map_.end())
			return;

		const auto& data = result->second;

		GL_CHECK(glDeleteBuffers(1, &data.buffer));
		texture_buffer_map_.erase(cmd.handle);

#ifdef _DEBUG
		Info("Texture buffer %d deleted", cmd.handle);
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
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.buffer));
		GL_CHECK(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GLintptr(cmd.offset), size, cmd.data.data()));

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

		const auto& result = constant_buffer_map_.find(cmd.handle);
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

}