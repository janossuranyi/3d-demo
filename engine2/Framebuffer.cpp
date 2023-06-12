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
		fb->AttachImageDepth( globalImages.Shadow );
		if (!fb->Check())
		{
			Error("[Framebuffer]: shadowFBO init failed!");
		}
		globalFramebuffers.shadowFBO = fb;

		for (int i = 0; i < 2; ++i)
		{
			fb = new Framebuffer("hdrBloom_"+std::to_string(i), w/2, h/2);
			fb->Bind();
			fb->AttachImage2D(globalImages.HDRbloom[i], 0);
			if (!fb->Check())
			{
				Error("[Framebuffer]: hdrBloom init failed!");
			}
			globalFramebuffers.bloomFBO[i] = fb;
		}

		fb = new Framebuffer( "hdrFBO", w, h );
		fb->Bind();
		fb->AttachImage2D( globalImages.HDRaccum, 0 );
		fb->AttachImageDepth( globalImages.HDRdepth );
		fb->AttachImageStencil(globalImages.HDRdepth);
		if ( ! fb->Check() )
		{
			Error("[Framebuffer]: hdrFBO init failed!");
		}
		globalFramebuffers.hdrFBO = fb;

		fb = new Framebuffer( "gbufferFBO", w, h );
		fb->Bind();
		fb->AttachImageDepth( globalImages.GBufferDepth );
		fb->AttachImageStencil(globalImages.GBufferDepth);
		fb->AttachImage2D( globalImages.GBufferAlbedo, 0 );
		fb->AttachImage2D( globalImages.GBufferNormal, 1);
		fb->AttachImage2D( globalImages.GBufferSpec, 2 );
		fb->AttachImage2D( globalImages.GBufferFragPos, 3);
		fb->AttachImage2D(globalImages.GBufferEmissive, 4);
		if ( ! fb->Check() )
		{
			Error("[Framebuffer]: GBufferFBO init failed!");
		}
		globalFramebuffers.GBufferFBO = fb;

		fb = new Framebuffer("defaultFBO", w, h);
		fb->Bind();
		fb->AttachImage2D(globalImages.defaultImage, 0);
		if (!fb->Check())
		{
			Error("[Framebuffer]: defaultFBO init failed!");
		}
		globalFramebuffers.defaultFBO = fb;

		Framebuffer::Unbind();
	}
	bool Framebuffer::IsDefaultFramebufferActive()
	{
		return (glcontext.currentFramebuffer == 0);
	}

}