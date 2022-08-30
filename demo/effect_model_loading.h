#pragma once
#include <SDL.h>
#include "effect.h"
#include "pipeline.h"
#include "gpu_buffer.h"
#include "gpu_program.h"
#include "mesh.h"
#include "world.h"

struct LoadModelEffect : public Effect
{
	// Wind turbine.glb
	// Steampunk_Dirigible_with_Ship.glb
	const std::string worldFile = "assets/Steampunk_Dirigible_with_Ship.glb";

	LoadModelEffect() :
		pipeline(),
		m_mesh() {}

	// Inherited via Effect
	virtual bool Init() override;
	virtual bool Update(float time) override;
	virtual bool HandleEvent(const SDL_Event* ev) override;
	virtual void Render() override;

	World world;
	Pipeline pipeline;
	RenderMesh3D m_mesh;
	GpuProgram shader;
	GpuTexture2D normalTex;
	float angleY{0.0f};

};