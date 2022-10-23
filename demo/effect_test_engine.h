#pragma once

#include <SDL.h>
#include "engine/gfx/gfx.h"
#include "effect.h"

struct EngineTestEffect : public Effect
{
	bool Init();
	bool Update(float time);
	bool HandleEvent(const SDL_Event* ev, float time);
	void Render();

	gfx::Renderer renderer;
	gfx::VertexBufferHandle vb;
	gfx::IndexBufferHandle ib;

};

