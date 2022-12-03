#pragma once
#include <SDL.h>
#include "effect.h"
#include "engine/gfx/renderer.h"
#include "engine/gfx/vertex_cache.h"

class BumpEffect : public Effect
{
public:
	// Inherited via Effect
	virtual bool Init() override;

	virtual bool Update(float time) override;

	virtual bool HandleEvent(const SDL_Event* ev, float time) override;

	virtual bool Render(uint64_t frame) override;
private:
	gfx::TextureHandle diffuse_;
	gfx::TextureHandle normal_;
	gfx::TextureHandle bump_;
	gfx::ProgramHandle shader_;
	gfx::UniformMap uniforms_;
	gfx::vtxCacheHandle vcache;
	gfx::vtxCacheHandle icache;

	vec2 yawPitch_;
};