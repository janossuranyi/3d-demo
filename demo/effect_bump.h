#pragma once
#include <SDL.h>
#include "effect.h"
#include "engine/gfx/renderer.h"
#include "engine/gfx/vertex_cache.h"
#include "engine/scene/camera.h"

class BumpEffect : public Effect
{
public:
	// Inherited via Effect
	virtual bool Init() override;

	virtual bool Update(float time) override;

	virtual bool HandleEvent(const SDL_Event* ev, float time) override;

	virtual bool Render(uint64_t frame) override;

	~BumpEffect();

private:
	gfx::TextureHandle diffuse_;
	gfx::TextureHandle normal_;
	gfx::TextureHandle bump_;
	gfx::ProgramHandle shader_;
	gfx::UniformMap uniforms_;
	gfx::vtxCacheHandle vcache;
	gfx::vtxCacheHandle icache;

	vec2 yawPitch_;
	vec3 rot_;
	vec3 lpos_;
	float lpower_{ 20.f };
	float vZ_;

	scene::Camera cam_{ vec3(0,1,2) };

};