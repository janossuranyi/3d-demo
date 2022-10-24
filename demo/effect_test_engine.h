#pragma once

#include <SDL.h>
#include "engine/gfx/gfx.h"
#include "effect.h"
#include "resource/filesystem.h"
#include "resource/resource_manager.h"


struct EngineTestEffect : public Effect
{
	~EngineTestEffect();
	bool Init();
	bool Update(float time);
	bool HandleEvent(const SDL_Event* ev, float time);
	void Render();

	gfx::Renderer renderer;
	gfx::VertexBufferHandle vb_points, vb_pp, vb_skybox;
	gfx::FrameBufferHandle fb;
	gfx::TextureHandle color_attachment;
	gfx::TextureHandle skybox;

	const int NUMPOINTS = 500000;
	float pp_offset{ 1.0f / 1000.0f };

	float rotX, rotY, eyeZ;
	glm::mat4 P;
	glm::mat4 VP;

	gfx::ProgramHandle prgPoints;
	gfx::ProgramHandle prgPP;
	gfx::ProgramHandle prgSkybox;

};

