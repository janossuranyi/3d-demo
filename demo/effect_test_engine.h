#pragma once

#include <SDL.h>
#include "engine/gfx/gfx.h"
#include "engine/gfx/vertex_cache.h"
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
	gfx::vtxCacheHandle vc_points, vc_pp, vc_skybox;

	gfx::VertexBufferHandle vb_points, vb_pp, vb_skybox;
	gfx::FrameBufferHandle fb;
	gfx::TextureHandle color_attachment, depth_attachment;
	gfx::TextureHandle skybox, texDyn;
	int kernel{ 0 };

	const int NUMPOINTS = 500000;
	float pp_offset{ 1.0f / 1000.0f };

	float rotX, rotY, eyeZ, angle;
	glm::mat4 P;
	glm::mat4 VP;

	gfx::ProgramHandle prgPoints;
	gfx::ProgramHandle prgPP;
	gfx::ProgramHandle prgSkybox;
	gfx::ProgramHandle prgDepth;
	gfx::ProgramHandle prgComp;
	gfx::ProgramHandle prgViewTex;
	gfx::VertexCache vtx_cache;
};

