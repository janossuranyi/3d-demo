#pragma once
#include <SDL.h>
#include <memory>
#include "effect.h"
#include "pipeline.h"
#include "gpu_buffer.h"
#include "gpu_framebuffer.h"
#include "gpu_program.h"
#include "mesh.h"
#include "world.h"

struct LoadModelEffect : public Effect
{
	// Wind turbine.glb
	// Steampunk_Dirigible_with_Ship.glb
	//const std::string worldFile = "assets/RedFox.glb";
	const std::string worldFile = "assets/Steampunk_Dirigible_with_Ship.glb";

	LoadModelEffect() :
		pipeline(),
		fb(),
		cb_color(),
		cb_color2(),
		cb_depth(),
		vertFormat(),
		fb_copy(),
		fb_blur(),
		cb_copy(),
		cb_blur(),
		m_mesh() {}

	// Inherited via Effect
	virtual bool Init() override;
	virtual bool Update(float time) override;
	virtual bool HandleEvent(const SDL_Event* ev, float time) override;
	virtual bool Render() override;

	World world;
	Pipeline pipeline;
	RenderMesh3D m_mesh;
	
	GpuProgram gauss;
	GpuProgram bloom_prepass;
	GpuProgram bloom;
	GpuProgram shader;
	GpuProgram fxaa;
	
	std::unique_ptr<GpuBuffer> rectBuffer;

	std::shared_ptr<GpuTexture2D> cb_color, cb_color2;
	std::shared_ptr<GpuTexture2D> cb_depth;

	GpuFrameBuffer fb;
	GpuFrameBuffer fb_copy;
	GpuFrameBuffer fb_blur[2];
	std::shared_ptr<GpuTexture2D> cb_blur[2];
	std::shared_ptr<GpuTexture2D> cb_copy;

	VertexLayout vertFormat;

	float angleY{0.0f};
	float posY{ 1.0f };
	float posZ{ 0.0f };
	float exposure{ 1.0f };

	void blur(int iter, int w, int h, int active_src = 0);
};