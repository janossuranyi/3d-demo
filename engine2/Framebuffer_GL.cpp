#include <GL/glew.h>
#include "./FrameBuffer.h"
#include "./RenderSystem.h"
#include "./RenderBackend_GL.h"
#include "./Logger.h"

namespace jsr {
	
	Framebuffer::Framebuffer(const std::string& name, int width, int height)
	{
		this->fboName = name;
		this->width = width;
		this->height = height;
		this->depthAttachmemt = -1;
		this->stencilAttachmemt = -1;
		this->apiObject = -1;

		for (int k = 0; k < JSR_MAX_COLOR_ATTACHMENTS; ++k) { this->colorAttachment[k] = -1; }

		GL_CHECK(glGenFramebuffers(1, (GLuint*)&apiObject));

		if (apiObject == -1) {
			Error("Cannot create GL frambuffer!");
		}

		framebuffers.push_back(this);
	}

	Framebuffer::~Framebuffer()
	{
		if (apiObject != -1)
		{
			GL_CHECK(glDeleteFramebuffers(1, (GLuint*)&apiObject));
		}
	}

	void Framebuffer::AttachImage2D(const Image* img, int index, int level, int cubeFace)
	{
		if (index >= JSR_MAX_COLOR_ATTACHMENTS) return;
		if (colorAttachment[index] != -1) return;
		if (img == nullptr || !img->IsCreated()) return;

		if (img->GetShape() == IMS_CUBEMAP)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeFace, img->apiObject, level);
		}
		else if (img->GetShape() == IMS_2D)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, img->apiObject, level);
		}
		else
		{
			Error("Invalid texture type for FBO color attachment!");
			return;
		}
		colorAttachment[index] = img->apiObject;
	}

	void Framebuffer::AttachImageDepth(const Image* img, int layer)
	{
		if (depthAttachmemt != -1) return;
		if (img == nullptr || !img->IsCreated()) return;

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, img->apiTarget, img->apiObject, layer);
		depthAttachmemt = img->apiObject;
	}

	void Framebuffer::AttachImageStencil(const Image* img)
	{
		if (stencilAttachmemt != -1) return;
		if (img == nullptr || !img->IsCreated()) return;

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, img->apiTarget, img->apiObject, 0);
		stencilAttachmemt = img->apiObject;
	}

	bool Framebuffer::Check()
	{
		int prev;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev);

		glBindFramebuffer(GL_FRAMEBUFFER, apiObject);

		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status == GL_FRAMEBUFFER_COMPLETE)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, prev);
			return true;
		}

		// something went wrong
		switch (status)
		{
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			Error("Framebuffer::Check( %s ): Framebuffer incomplete, incomplete attachment", fboName.c_str());
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			Error("Framebuffer::Check( %s ): Framebuffer incomplete, missing attachment", fboName.c_str());
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			Error("Framebuffer::Check( %s ): Framebuffer incomplete, missing draw buffer", fboName.c_str());
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			Error("Framebuffer::Check( %s ): Framebuffer incomplete, missing read buffer", fboName.c_str());
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			Error("Framebuffer::Check( %s ): Framebuffer incomplete, missing layer targets", fboName.c_str());
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			Error("Framebuffer::Check( %s ): Framebuffer incomplete, missing multisample", fboName.c_str());
			break;

		case GL_FRAMEBUFFER_UNSUPPORTED:
			Error("Framebuffer::Check( %s ): Unsupported framebuffer format", fboName.c_str());
			break;

		default:
			Error("Framebuffer::Check( %s ): Unknown error 0x%X", fboName.c_str(), status);
			break;
		};

		glBindFramebuffer(GL_FRAMEBUFFER, prev);

		return false;
	}

	void Framebuffer::Bind()
	{
		if (renderSystem.backend->currentFramebuffer != this)
		{
			GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, apiObject));
			renderSystem.backend->currentFramebuffer = this;
		}
	}

	bool Framebuffer::IsBound() const
	{
		return renderSystem.backend->currentFramebuffer == this;
	}

	void Framebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		renderSystem.backend->currentFramebuffer = NULL;
	}

}