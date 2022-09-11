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
		fb_color(),
		fb_depth(),
		vertFormat(),
		m_mesh() {}

	// Inherited via Effect
	virtual bool Init() override;
	virtual bool Update(float time) override;
	virtual bool HandleEvent(const SDL_Event* ev, float time) override;
	virtual void Render() override;

	World world;
	Pipeline pipeline;
	RenderMesh3D m_mesh;
	GpuProgram shader;
	GpuProgram fxaa;
	std::unique_ptr<GpuBuffer> rectBuffer;

	GpuTexture2D::Ptr fb_color;
	GpuTexture2D::Ptr fb_depth;
	GpuFrameBuffer fb;

	VertexLayout vertFormat;

	float angleY{0.0f};
	float posY{ 1.0f };
	float posZ{ 0.0f };

};