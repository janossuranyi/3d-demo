#include "./RenderSystem.h"
#include "./ImageManager.h"
#include "./FrameBuffer.h"
#include "./Logger.h"

namespace jsr {

	std::vector<Framebuffer*>	Framebuffer::framebuffers;
	globalFramebuffers_t globalFramebuffers;

	void Framebuffer::Shutdown()
	{
		for (auto* fbo : framebuffers)
		{
			delete fbo;
		}

		framebuffers.clear();
	}
	void Framebuffer::Init()
	{
		// Creating global framebuffers
		Framebuffer* fb;
		ImageManager* imgr = renderSystem.imageManager;

		int w, h;
		renderSystem.backend->GetScreenSize( w, h );

		const int shadowRes = renderGlobals.shadowResolution;

		fb = new Framebuffer("shadowFBO", shadowRes, shadowRes);
		fb->Bind();
		fb->AttachImageDepth( imgr->globalImages.Depth32 );
		if (!fb->Check())
		{
			Error("[Framebuffer]: shadowFBO init failed!");
		}
		globalFramebuffers.shadowFBO = fb;

		fb = new Framebuffer( "defaultFBO", w, h);
		fb->Bind();
		fb->AttachImage2D(imgr->globalImages.defaultImage, 0);
		fb->AttachImageDepth( imgr->globalImages.defaultDepth );
		if ( ! fb->Check() )
		{
			Error("[Framebuffer]: defaultFBO init failed!");
		}
		globalFramebuffers.defaultFBO = fb;

		fb = new Framebuffer( "hdrFBO", w, h );
		fb->Bind();
		fb->AttachImage2D( imgr->globalImages.HDRaccum, 0 );
		fb->AttachImageDepth(imgr->globalImages.HDRdepth);
		if ( ! fb->Check() )
		{
			Error("[Framebuffer]: hdrFBO init failed!");
		}
		globalFramebuffers.hdrFBO = fb;

		fb = new Framebuffer( "gbufferFBO", w, h );
		fb->Bind();
		fb->AttachImageDepth( imgr->globalImages.Depth32 );
		fb->AttachImage2D( imgr->globalImages.GBufferAlbedo, 0 );
		fb->AttachImage2D( imgr->globalImages.GBufferNormal, 1);
		fb->AttachImage2D( imgr->globalImages.GBufferSpec, 2 );
		fb->AttachImage2D( imgr->globalImages.GBufferFragPos, 3);
		if ( ! fb->Check() )
		{
			Error("[Framebuffer]: GBufferFBO init failed!");
		}
		globalFramebuffers.GBufferFBO = fb;

		Framebuffer::Unbind();
	}
	bool Framebuffer::IsDefaultFramebufferActive()
	{
		return (renderSystem.backend->currentFramebuffer == NULL);
	}

}