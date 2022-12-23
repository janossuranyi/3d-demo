#include <GL/glew.h>
#include "gfx/opengl/gl_context.h"
#include "gfx/opengl/gl_helper.h"

#include "logger.h"

#define USE_MAP_BUFFER 0

namespace gfx {

	GLuint OpenGLRenderContext::create_buffer_real(GLenum target, BufferUsage usage, uint size, const Memory data, uint& actualSize)
	{
		GLuint buffer = static_cast<GLuint>(0xffff);
		const GLenum _usage = MapBufferUsage(usage);


		const GLsizeiptr _size = std::max(data.size(), size_t(size));

		assert(ext_.EXT_direct_state_access);

		GL_CHECK(glGenBuffers(1, &buffer));
		GL_CHECK(glNamedBufferDataEXT(buffer, _size, nullptr, _usage));
		if (!data.empty())
		{
			GL_CHECK(glNamedBufferSubDataEXT(buffer, 0, data.size(), data.data()));
		}
		actualSize = _size;
		return buffer;
	}

	void OpenGLRenderContext::update_buffer_real(GLenum target, GLuint buffer, uint offset, uint pixelByteSize, const Memory data)
	{
		const GLsizeiptr size_ = pixelByteSize > 0 ? GLsizeiptr(pixelByteSize) : data.size();

		GL_CHECK(glNamedBufferSubDataEXT(buffer, GLintptr(offset), GLsizeiptr(size_), data.data()));
	}

	void OpenGLRenderContext::operator()(const cmd::CreateShaderStorageBuffer& cmd)
	{

		if (shader_buffer_map_.count(cmd.handle) > 0)
			return;

		assert(cmd.size < gl_max_shader_storage_block_size_ && cmd.data.size() < gl_max_shader_storage_block_size_);

		uint _size = cmd.size;
		const GLuint buffer = create_buffer_real(GL_SHADER_STORAGE_BUFFER, cmd.usage, cmd.size, cmd.data, _size);

		shader_buffer_map_.emplace(cmd.handle, ShaderBufferData{ buffer, _size, cmd.usage });

	}

	void OpenGLRenderContext::operator()(const cmd::UpdateShaderStorageBuffer& cmd)
	{
		if (cmd.data.data() == nullptr)
			return;

		const auto result = shader_buffer_map_.find(cmd.handle);
		if (result == shader_buffer_map_.end())
			return;

		const auto& data = result->second;

		update_buffer_real(GL_SHADER_STORAGE_BUFFER, data.buffer, cmd.offset, cmd.size, cmd.data);

	}

	void OpenGLRenderContext::operator()(const cmd::DeleteShaderStorageBuffer& cmd)
	{
		const auto result = shader_buffer_map_.find(cmd.handle);
		if (result == shader_buffer_map_.end())
			return;

		const auto& data = result->second;

		GL_CHECK(glDeleteBuffers(1, &data.buffer));
		shader_buffer_map_.erase(cmd.handle);

#ifdef _DEBUG
		Info("Shader storage buffer %d deleted", cmd.handle);
#endif
	}

	void OpenGLRenderContext::operator()(const cmd::CreateVertexBuffer& cmd)
	{
		if (vertex_buffer_map_.count(cmd.handle) > 0)
			return;

		uint _size = cmd.size;
		const GLuint buffer = create_buffer_real(GL_ARRAY_BUFFER, cmd.usage, cmd.size, cmd.data, _size);

		vertex_buffer_map_.emplace(cmd.handle, VertexBufferData{ buffer, _size, cmd.usage });
	}

	void OpenGLRenderContext::operator()(const cmd::CreateTextureBuffer& cmd)
	{
		if (texture_buffer_map_.count(cmd.handle) > 0)
			return;

		uint _size = cmd.size;
		const GLuint buffer = create_buffer_real(GL_TEXTURE_BUFFER, cmd.usage, cmd.size, cmd.data, _size);

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
		update_buffer_real(GL_TEXTURE_BUFFER, data.buffer, cmd.offset, cmd.size, cmd.data);
	}

	void OpenGLRenderContext::operator()(const cmd::UpdateVertexBuffer& cmd)
	{
		if (cmd.data.data() == nullptr)
			return;

		const auto result = vertex_buffer_map_.find(cmd.handle);
		if (result == vertex_buffer_map_.end())
			return;

		const auto& data = result->second;
		update_buffer_real(GL_ARRAY_BUFFER, data.buffer, cmd.offset, cmd.size, cmd.data);
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

		uint _size = cmd.size;
		const GLuint buffer = create_buffer_real(GL_ELEMENT_ARRAY_BUFFER, cmd.usage, cmd.size, cmd.data, _size);

		index_buffer_map_.emplace(cmd.handle, IndexBufferData{ buffer, _size, cmd.usage, cmd.type });

	}

	void OpenGLRenderContext::operator()(const cmd::UpdateIndexBuffer& cmd)
	{
		if (cmd.data.data() == nullptr)
			return;

		const auto result = index_buffer_map_.find(cmd.handle);

		if (result != std::end(index_buffer_map_))
		{
			const auto& data = result->second;
			update_buffer_real(GL_ELEMENT_ARRAY_BUFFER, data.buffer, cmd.offset, 0, cmd.data);
		}
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

		assert(cmd.size < gl_max_uniform_block_size_ && cmd.data.size() < gl_max_uniform_block_size_);

		uint _size = cmd.size;
		const GLuint buffer = create_buffer_real(GL_UNIFORM_BUFFER, cmd.usage, cmd.size, cmd.data, _size);

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
		update_buffer_real(GL_UNIFORM_BUFFER, data.buffer, cmd.offset, cmd.size, cmd.data);
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