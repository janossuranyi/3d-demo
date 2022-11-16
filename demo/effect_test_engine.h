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
	bool Render(uint64_t frame);

	gfx::vtxCacheHandle vc_points, vc_skybox;
	gfx::VertexCache vtx_cache;

	gfx::VertexBufferHandle vb_points, vb_skybox;
	gfx::VertexBufferHandle tmp;
	gfx::FrameBufferHandle fb;
	gfx::TextureBufferHandle texBuf;

	gfx::TextureHandle color_attachment, depth_attachment;
	gfx::TextureHandle skybox, texDyn, bufTex;
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
	gfx::ProgramHandle prgGauss;
	gfx::VertexLayoutHandle layout_handle;

	struct frameData_t {
		gfx::UniformMap effect1_vars;
		gfx::UniformMap effect2_vars;
		gfx::UniformMap effect3_vars;
		gfx::UniformMap effect4_vars;
		gfx::UniformMap effect5_vars;
		gfx::UniformMap effect6_vars;
		gfx::VertexAttribMap attrs{ {5,2U} };
	} perframe_data[2];
};

