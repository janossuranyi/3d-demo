#include "./RenderSystem.h"
#include "./FrameBuffer.h"

namespace jsr {

	std::list<Framebuffer*>	Framebuffer::framebuffers;
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
	}
	bool Framebuffer::IsDefaultFramebufferActive()
	{
		return (renderSystem.backend->currentFramebuffer == NULL);
	}

}