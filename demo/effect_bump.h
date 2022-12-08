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

	struct Light {
		vec4 pos;
		vec4 color;
	};

	struct LightInfoBlock
	{
		Light	g_lights[256];
	};

private:
	gfx::TextureHandle diffuse_;
	gfx::TextureHandle normal_;
	gfx::TextureHandle bump_;
	gfx::ProgramHandle shader_;
	gfx::UniformMap uniforms_;
	gfx::vtxCacheHandle vcache;
	gfx::vtxCacheHandle icache;
	gfx::ConstantBufferHandle lightInfoBuffer_;

	vec2 yawPitch_;
	vec3 rot_;
	vec3 lpos_{0};

	int numLights_{ 0 };
	LightInfoBlock lightInfo_{};

	float lpower_{};
	float vZ_;

	scene::Camera cam_{ vec3(0,1,2) };

};