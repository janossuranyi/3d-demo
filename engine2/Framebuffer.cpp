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
		int bloomDiv = 1 << renderGlobals.bloomDownsampleLevel;

		for (int i = 0; i < 2; ++i)
		{
			fb = new Framebuffer("hdrBlure_" + std::to_string(i), w / bloomDiv, h / bloomDiv);
			fb->Bind();
			fb->AttachImage2D(globalImages.HDRblur[i], 0);
			if (!fb->Check())
			{
				Error("[Framebuffer]: hdrBlure init failed!");
			}
			globalFramebuffers.blurFBO[i] = fb;
		}

		int w2 = w;
		int h2 = h;
		globalFramebuffers.bloomFBO.resize(renderGlobals.bloomDownsampleLevel);
		for (int i = 0; i < renderGlobals.bloomDownsampleLevel; ++i)
		{
			w2 /= 2;
			h2 /= 2;
			fb = new Framebuffer("hdrBloom_" + std::to_string(i), w2, h2);
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