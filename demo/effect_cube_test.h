#pragma once

#include <SDL.h>
#include "effect.h"
#include "engine/gfx/renderer.h"

class CubeDemoEffect : public Effect
{
public:
	// Inherited via Effect
	virtual bool Init() override;
	virtual bool Update(float time) override;
	virtual bool HandleEvent(const SDL_Event* ev, float time) override;
	virtual bool Render(uint64_t frame) override;
private:
	gfx::TextureHandle cubemap_;
	gfx::FrameBufferHandle offline_fb_;

};