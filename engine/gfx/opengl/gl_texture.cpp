#include "gfx/opengl/gl_context.h"
#include "gfx/opengl/gl_helper.h"

#include "logger.h"

namespace gfx {

	void OpenGLRenderContext::operator()(const cmd::CreateBufferTexture& cmd)
	{
		if (texture_map_.count(cmd.htexture))
		{
			return;
		}

		auto& texBuf = texture_buffer_map_.find(cmd.hbuffer);
		if (texBuf == std::end(texture_buffer_map_))
		{
			return;
		}

		GLuint texture;
		const GLenum target = GL_TEXTURE_BUFFER;
		GL_CHECK(glGenTextures(1, &texture));
		GL_CHECK(glBindTexture(target, texture));
		auto& texinfo = s_texture_format[static_cast<int>(cmd.format)];
		if (cmd.size == 0) {
			glTexBuffer(target, texinfo.internal_format, texBuf->second.buffer);
		}
		else {
			assert((cmd.offset & (gl_texture_buffer_offset_alignment_ - 1)) == 0);
			glTexBufferRange(target, texinfo.internal_format, texBuf->second.buffer, cmd.offset, cmd.size);
		}

		texture_map_.emplace(cmd.htexture, TextureData{ texture, target, cmd.format });
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
		if (texture_map_.count(cmd.handle))
		{
			return;
		}

		GLuint texture;
		const GLenum target = GL_TEXTURE_2D;
		GL_CHECK(glGenTextures(1, &texture));
		GL_CHECK(glBindTexture(target, texture));
		auto& texinfo = s_texture_format[static_cast<int>(cmd.format)];

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

		GL_CHECK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filter));
		GL_CHECK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag_filter));
		GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap));
		GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap));

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
		const auto& texinfo = s_texture_format[static_cast<size_t>(cmd.format)];
		const GLenum wrap = s_texture_wrap_map.at(cmd.wrap);
		const GLenum min_filter = s_texture_filter_map.at(cmd.min_filter);
		const GLenum mag_filter = s_texture_filter_map.at(cmd.mag_filter);

		GL_CHECK(glGenTextures(1, &texture));
		GL_CHECK(glBindTexture(target, texture));

		const GLsizei levels = static_cast<GLsizei>(std::floorf(std::log2f(static_cast<float>(cmd.width)))) + 1;

		GL_CHECK(glTexStorage2D(target,
			levels,
			(cmd.srgb ? texinfo.internal_format_srgb : texinfo.internal_format),
			cmd.width,
			cmd.height));

		for (unsigned int face = 0; face < 6; ++face)
		{
			/*
			GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
				(cmd.srgb ? texinfo.internal_format_srgb : texinfo.internal_format),
				cmd.width,
				cmd.height,
				0,
				texinfo.format,
				texinfo.type,
				cmd.data[face].data()));
				*/
			GL_CHECK(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
				/*level*/	0,
				/*xoffset*/	0,
				/*yoffset*/	0,
				cmd.width,
				cmd.height,
				texinfo.format,
				texinfo.type,
				cmd.data[face].data()));
		}

		GL_CHECK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filter));
		GL_CHECK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag_filter));
		GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap));
		GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap));
		GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_R, wrap));
		if (cmd.mipmap)
		{
			GL_CHECK(glGenerateMipmap(target));
		}
		GL_CHECK(glBindTexture(target, 0));

		const TextureData t_data{ texture, target };
		texture_map_.emplace(cmd.handle, t_data);
	}

}