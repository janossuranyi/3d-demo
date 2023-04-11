#ifndef JSR_FRAMEBUFFER_H
#define JSR_FRAMEBUFFER_H

#include <string>
#include <list>

#define JSR_MAX_COLOR_ATTACHMENTS 16

namespace jsr {

	class Image;
	class Framebuffer
	{
	public:
		Framebuffer(const std::string& name, int width, int height);
		static void Shutdown();
		static void Init();
		virtual ~Framebuffer();
		void	AttachImage2D(const Image* img, int index, int level = 0, int cubeFace = 0);
		void	AttachImageDepth(const Image* img, int layer = 0);
		void	AttachImageStencil(const Image* img);
		bool	Check();
		void	Bind();
		bool	IsBound() const;
		static void Unbind();
		static bool IsDefaultFramebufferActive();
	private:
		std::string		fboName;
		int				width;
		int				height;
		int				colorAttachment[JSR_MAX_COLOR_ATTACHMENTS];
		int				depthAttachmemt;
		int				stencilAttachmemt;
		int				apiObject;

		static std::list<Framebuffer*>	framebuffers;
	};

	struct globalFramebuffers_t
	{
		Framebuffer*	shadowMapFBO;
		Framebuffer*	hdrFBO;
		Framebuffer*	GBuffer0;
		Framebuffer*	GBuffer1;
		Framebuffer*	depthFBO;
	};

	extern globalFramebuffers_t globalFramebuffers;
}
#endif // !JSR_FRAMEBUFFER_H
