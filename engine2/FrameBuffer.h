#ifndef JSR_FRAMEBUFFER_H
#define JSR_FRAMEBUFFER_H

#include <string>
#include <list>


namespace jsr {

	const int MAX_COLOR_ATTACHMENTS =16;

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
		int				colorAttachments[ MAX_COLOR_ATTACHMENTS ];
		int				depthAttachment;
		int				stencilAttachment;
		int				apiObject;

		static std::list<Framebuffer*>	framebuffers;
	};

	struct globalFramebuffers_t
	{
		Framebuffer*	shadowFBO;
		Framebuffer*	hdrFBO;
		Framebuffer*	GBufferFBO;
		Framebuffer*	defaultFBO;
	};

	extern globalFramebuffers_t globalFramebuffers;
}
#endif // !JSR_FRAMEBUFFER_H
