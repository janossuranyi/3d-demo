#include "gfx/opengl/gl_context.h"
#include "gfx/opengl/gl_helper.h"
#include "logger.h"

namespace gfx {
	void OpenGLRenderContext::operator()(const cmd::CreateOrUpdateFramebuffer& cmd)
	{

		FrameBufferData fb_data{};

		if (frame_buffer_map_.count(cmd.handle) > 0)
		{
			fb_data = frame_buffer_map_.at(cmd.handle);
		}
		else
		{
			GL_CHECK(glGenFramebuffers(1, &fb_data.frame_buffer));
		}

		ushort old_width = fb_data.width, old_height = fb_data.height;

		fb_data.width = cmd.width;
		fb_data.height = cmd.height;

		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fb_data.frame_buffer));
		std::vector<GLenum> draw_buffers;
		int attachment{ 0 };
		for (auto& fb_texture : cmd.textures)
		{
			const auto& gl_texture = texture_map_.find(fb_texture.handle);
			assert(gl_texture != texture_map_.end());

			draw_buffers.emplace_back(GL_COLOR_ATTACHMENT0 + attachment++);
			GLuint target{};
			if (gl_texture->second.target == GL_TEXTURE_2D)
			{
				target = GL_TEXTURE_2D;
			}
			else if (gl_texture->second.target == GL_TEXTURE_CUBE_MAP)
			{
				target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + fb_texture.face;
			}
			GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, draw_buffers.back(), target, gl_texture->second.texture, fb_texture.level));
		}

		if ( ! cmd.textures.empty() )
		{
			GL_CHECK( glDrawBuffers(static_cast<GLsizei>(draw_buffers.size()), draw_buffers.data()) );
		}

		if (cmd.depth_stencil_texture.isValid())
		{
			auto& depth = texture_map_.at(cmd.depth_stencil_texture);
			// Stencil only
			if (depth.format == TextureFormat::D0S8)
			{
				GL_CHECK(
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth.texture, 0));
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
				GL_CHECK(
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth.texture, 0));
			}
			else // depth+stencil
			{
				GL_CHECK(
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth.texture, 0));
			}
		}
		else if (old_height != fb_data.height || old_width != fb_data.width)
		{
			if (!fb_data.depth_render_buffer)
			{
				// Create depth buffer.
				GL_CHECK( glGenRenderbuffers(1, &fb_data.depth_render_buffer) );
			}
			GL_CHECK(
				glBindRenderbuffer(GL_RENDERBUFFER, fb_data.depth_render_buffer));
			GL_CHECK(
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, cmd.width, cmd.height));
			GL_CHECK(
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
				fb_data.depth_render_buffer));
		}

		// Check frame buffer status.
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER); GLC();
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			Error("[CreateFrameBuffer] The framebuffer is not complete. Status: 0x%x", status);
		}

		// Unbind.
		GL_CHECK(
			glBindFramebuffer(GL_FRAMEBUFFER, 0));

		// Add to map.
		frame_buffer_map_.emplace(cmd.handle, fb_data);
	}

	void OpenGLRenderContext::operator()(const cmd::DeleteFramebuffer& cmd)
	{
		if (frame_buffer_map_.count(cmd.handle) == 0)
		{
			return;
		}

		auto& fbData = frame_buffer_map_.at(cmd.handle);
		GL_CHECK(
			glDeleteFramebuffers(1, &fbData.frame_buffer));
		GL_CHECK(
			glDeleteRenderbuffers(1, &fbData.depth_render_buffer));
#ifdef _DEBUG
		Info("Framebuffer %d deleted", cmd.handle);
#endif
	}

}