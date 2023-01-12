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
		GL_CHECK(glCreateBuffers(1, &buffer));
//		GL_CHECK(glNamedBufferData(buffer, _size, nullptr, _usage));
		GL_CHECK(glNamedBufferStorage(buffer, _size, nullptr, GL_DYNAMIC_STORAGE_BIT|GL_MAP_WRITE_BIT| GL_MAP_PERSISTENT_BIT| GL_MAP_COHERENT_BIT));
		if (!data.empty())
		{
			GL_CHECK(glNamedBufferSubData(buffer, 0, data.size(), data.data()));
		}
		actualSize = _size;
		return buffer;
	}

	void OpenGLRenderContext::update_buffer_real(GLenum target, GLuint buffer, uint offset, uint pixelByteSize, const Memory data)
	{
		const GLsizeiptr size_ = pixelByteSize > 0 ? GLsizeiptr(pixelByteSize) : data.size();

		void* ptr{};
		GL_CHECK(ptr = glMapNamedBufferRange(buffer, offset, size_, GL_MAP_WRITE_BIT));
		if (ptr)
		{
			std::memcpy(ptr, data.data(), size_);
			GL_CHECK(glUnmapNamedBuffer(buffer));
		}
		else {
			GL_CHECK(glNamedBufferSubData(buffer, GLintptr(offset), GLsizeiptr(size_), data.data()));
		}
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

	void OpenGLRenderContext::operator()(const cmd::UpdateBuffer& cmd)
	{
		if (cmd.data.data() == nullptr)
			return;

		const auto result = buffer_data_map_.find(cmd.handle);
		if (result == buffer_data_map_.end())
			return;

		const auto& data = result->second;
		glNamedBufferSubData(data.buffer, cmd.offset, cmd.data.size(), cmd.data.data());
	}

	void OpenGLRenderContext::operator()(const cmd::DeleteBuffer& cmd)
	{
		const auto result = buffer_data_map_.find(cmd.handle);
		if (result == std::end(buffer_data_map_ ))
			return;

		const auto& data = result->second;

		GL_CHECK(glDeleteBuffers(1, &data.buffer));

		std::lock_guard<Mutex> lck(buffer_data_mutex_);
		buffer_data_map_.erase(cmd.handle);

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


	void OpenGLRenderContext::operator()(const cmd::CreateConstantBuffer& cmd)
	{
		if (constant_buffer_map_.count(cmd.handle) > 0)
			return;

		assert(cmd.size < gl_max_uniform_block_size_ && cmd.data.size() < gl_max_uniform_block_size_);

		uint _size = cmd.size;
		const GLuint buffer = create_buffer_real(GL_UNIFORM_BUFFER, cmd.usage, cmd.size, cmd.data, _size);
		
		void* addr{};
		GL_CHECK(addr = glMapNamedBufferRange(buffer, 0, _size, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT| GL_MAP_COHERENT_BIT));
		constant_buffer_map_.emplace(cmd.handle, ConstantBufferData{ buffer, _size, cmd.usage, addr });
	}

	void OpenGLRenderContext::operator()(const cmd::UpdateConstantBuffer& cmd)
	{
		if (cmd.data.data() == nullptr)
			return;

		const auto result = constant_buffer_map_.find(cmd.handle);
		if (result == std::end(constant_buffer_map_))
			return;

		const auto& data = result->second;
		if (data.mapped_address)
		{
			std::memcpy(reinterpret_cast<char*>( data.mapped_address ) + cmd.offset, cmd.data.data(), cmd.size > 0 ? cmd.size : cmd.data.size());
			//GL_CHECK(glFlushMappedNamedBufferRange(data.buffer, cmd.position, cmd.size > 0 ? cmd.size : cmd.data.size()));
		}
		else
		{
			update_buffer_real(GL_UNIFORM_BUFFER, data.buffer, cmd.offset, cmd.size, cmd.data);
		}
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

	void gfx::OpenGLRenderContext::operator()(const cmd::CreateBuffer& cmd)
	{
		auto find = buffer_data_map_.find(cmd.handle);

		// buffer exists
		if (find != std::end(buffer_data_map_)) return;

		BufferData bufferData{};

		GLbitfield flags{};
		{
			if (cmd.flags & eStorageFlags::MAP_READ_BIT)		flags |= GL_MAP_READ_BIT;
			if (cmd.flags & eStorageFlags::MAP_WRITE_BIT)		flags |= GL_MAP_WRITE_BIT;
			if (cmd.flags & eStorageFlags::MAP_COHERENT_BIT)	flags |= GL_MAP_COHERENT_BIT;
			if (cmd.flags & eStorageFlags::MAP_PERSISTENT_BIT)	flags |= GL_MAP_PERSISTENT_BIT;
			if (cmd.flags & eStorageFlags::DYNAMIC_BIT)			flags |= GL_DYNAMIC_STORAGE_BIT;
		}

		const GLsizeiptr _size = std::max(cmd.data.size(), size_t(cmd.size));

		GL_CHECK(glCreateBuffers(1, &bufferData.buffer));
		GL_CHECK(glNamedBufferStorage(bufferData.buffer, _size, nullptr, flags));
		if (!cmd.data.empty()) {
			GL_CHECK(glNamedBufferSubData(bufferData.buffer, cmd.bufferOffset, cmd.data.size(), cmd.data.data()));
		}

		if (cmd.flags & eStorageFlags::MAP_PERSISTENT_BIT) {
			GL_CHECK(bufferData.mapptr = glMapNamedBufferRange(bufferData.buffer, 0, _size, flags));
		}
		bufferData.size = _size;
		switch (cmd.target) {
		case BufferTarget::Vertex:
			bufferData.target = GL_ARRAY_BUFFER;
			break;
		case BufferTarget::Index:
			bufferData.target = GL_ELEMENT_ARRAY_BUFFER;
			break;
		case BufferTarget::Texture:
			bufferData.target = GL_TEXTURE_BUFFER;
			break;
		case BufferTarget::Uniform:
			bufferData.target = GL_UNIFORM_BUFFER;
			break;
		case BufferTarget::ShaderStorage:
			bufferData.target = GL_SHADER_STORAGE_BUFFER;
			break;
		}

		std::unique_lock<Mutex> lck(buffer_data_mutex_);
		buffer_data_map_.emplace(cmd.handle, bufferData);
	}
}