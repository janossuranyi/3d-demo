#include "gfx/opengl/gl_context.h"
#include "gfx/opengl/gl_helper.h"

#include "logger.h"

namespace gfx {

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

		GL_CHECK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filter));
		GL_CHECK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag_filter));
		GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap));
		GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap));
		if (cmd.mipmap)
		{
			GL_CHECK(glGenerateMipmap(target));
		}
		GL_CHECK(glBindTexture(target, 0));

		const TextureData t_data{ texture, target };
		texture_map_.emplace(cmd.handle, t_data);
	}

}