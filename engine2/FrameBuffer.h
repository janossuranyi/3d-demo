#ifndef JSR_FRAMEBUFFER_H
#define JSR_FRAMEBUFFER_H

#include <glm/glm.hpp>
#include <string>
#include <vector>


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
		int		HasColorAttachment() const;
		void	AttachImage2D(const Image* img, int index, int level = 0, int cubeFace = 0);
		void	AttachImageDepth(const Image* img, int layer = 0, int level = 0);
		void	AttachImageStencil(const Image* img);
		bool	Check();
		void	Bind();
		void	ClearAttachment(int i, const glm::vec4& color) const;
		void	BlitColorBuffer(int srcX1, int srcY1, int srcX2, int srcY2, int dstX1, int dstY1, int dstX2, int dstY2);
		void	BlitDepthBuffer(int srcX1, int srcY1, int srcX2, int srcY2, int dstX1, int dstY1, int dstX2, int dstY2);
		void	BindForReading();
		void	SetReadBuffer(int index) const;
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

		static std::vector<Framebuffer*>	framebuffers;
	};

	struct globalFramebuffers_t
	{
		Framebuffer*	shadowFBO;
		Framebuffer*	hdrFBO;
		Framebuffer*	bloomFBO[2];
		Framebuffer*	GBufferFBO;
		Framebuffer*	defaultFBO;
	};

	extern globalFramebuffers_t globalFramebuffers;
}
#endif // !JSR_FRAMEBUFFER_H
