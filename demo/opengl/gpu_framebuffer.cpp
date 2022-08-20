#include <GL/glew.h>

#include <cassert>
#include "logger.h"
#include "gpu_framebuffer.h"
#include "gpu_types.h"
#include "gpu_utils.h"

GpuFrameBuffer::~GpuFrameBuffer()
{
	if (!m_fbo) return;

	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	GL_CHECK(glDeleteFramebuffers(1, &m_fbo));
	if (m_depthRenderBuffer) GL_CHECK(glDeleteRenderbuffers(1, &m_depthRenderBuffer));

	if (m_renderBuffers.size() > 0)
	{
		GL_CHECK(glDeleteRenderbuffers(m_renderBuffers.size(), m_renderBuffers.data()));
	}
}

GpuFrameBuffer& GpuFrameBuffer::create()
{
	assert(m_fbo == 0);
	
	GL_CHECK(glGenFramebuffers(1, &m_fbo));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));

	return *this;
}

GpuFrameBuffer& GpuFrameBuffer::addColorAttachment(int index, std::shared_ptr<GpuTexture2D> texture)
{
	GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, texture->mTexture, 0));
	m_textures.push_back(texture);

	return *this;
}

GpuFrameBuffer& GpuFrameBuffer::addColorAttachment(int index, std::shared_ptr<GpuTextureCubeMap> texture)
{
	GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_CUBE_MAP, texture->mTexture, 0));
	m_textures.push_back(texture);
	
	return *this;
}

GpuFrameBuffer& GpuFrameBuffer::addColorAttachment(int index, int w, int h, ePixelFormat format)
{
	GLuint rbo;
	GLenum rb_format = GL_castPixelFormat(format);

	GL_CHECK(glGenRenderbuffers(1, &rbo));
	GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, rbo));
	GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, rb_format, w, h));
	GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, 0));

	GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, rbo));
	m_renderBuffers.push_back(rbo);

	return *this;
}

GpuFrameBuffer& GpuFrameBuffer::setDepthStencilAttachment(int w, int h)
{
	assert(m_depthRenderBuffer == 0 && m_depthRenderTexture == nullptr);

	GLuint rbo;
	GL_CHECK(glGenRenderbuffers(1, &rbo));
	GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, rbo));
	GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h));
	GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, 0));

	GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo));

	m_depthRenderBuffer = rbo;

	return *this;
}

GpuFrameBuffer& GpuFrameBuffer::setDepthStencilAttachment(std::shared_ptr<GpuTexture2D> texture)
{
	assert(m_depthRenderBuffer == 0 && m_depthRenderTexture == nullptr);

	GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture->mTexture, 0));
	m_depthRenderTexture = texture;

	return *this;
}

bool GpuFrameBuffer::checkCompletness()
{
	GLenum result;
	GL_CHECK(result = glCheckFramebufferStatus(GL_FRAMEBUFFER));

	if (result != GL_FRAMEBUFFER_COMPLETE)
	{
		char* message = "unknown framebuffer error";

		switch (result)
		{
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			message = "INCOMPLETE ATTACHMENT";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			message = "MISSING ATTACHMENT";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			message = "INCOMPLETE DRAW_BUFFER";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			message = "INCOMPLETE READ_BUFFER";
			break;
		}
		Error("ERROR::FRAMEBUFFER:: %s!", message);
		return false;
	}

	return true;
}

void GpuFrameBuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

