#pragma once

#include <SDL.h>
#include "engine/gfx/gfx.h"
#include "engine/gfx/vertex_cache.h"
#include "draw_vert.h"
#include "effect.h"
#include "resource/filesystem.h"
#include "resource/resource_manager.h"


struct EngineTestEffect : public Effect
{
	~EngineTestEffect();
	bool Init();
	bool Update(float time);
	bool HandleEvent(const SDL_Event* ev, float time);
	bool Render();

	gfx::vtxCacheHandle vc_points, vc_pp, vc_skybox;
	gfx::VertexCache vtx_cache;

	gfx::VertexBufferHandle vb_points, vb_pp, vb_skybox;
	gfx::VertexBufferHandle tmp;
	gfx::FrameBufferHandle fb;
	gfx::TextureBufferHandle tb;

	gfx::TextureHandle color_attachment, depth_attachment;
	gfx::TextureHandle skybox, texDyn, bt;
	int kernel{ 0 };

	const int NUMPOINTS = 500000;
	float pp_offset{ 1.0f / 1000.0f };

	float rotX, rotY, eyeZ, angle;
	glm::mat4 P;
	glm::mat4 VP;

	gfx::VertexDecl layout;
	gfx::ProgramHandle prgPoints;
	gfx::ProgramHandle prgPP;
	gfx::ProgramHandle prgSkybox;
	gfx::ProgramHandle prgDepth;
	gfx::ProgramHandle prgComp;
	gfx::ProgramHandle prgViewTex;
	gfx::VertexLayoutHandle layout_handle;
};

