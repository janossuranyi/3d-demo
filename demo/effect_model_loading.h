#pragma once
#include <SDL.h>
#include "effect.h"
#include "pipeline.h"
#include "gpu_buffer.h"
#include "mesh.h"

struct LoadModelEffect : public Effect
{
	LoadModelEffect() :
		pipeline(),
		m_mesh() {}

	// Inherited via Effect
	virtual bool Init() override;
	virtual bool Update(float time) override;
	virtual bool HandleEvent(const SDL_Event* ev) override;
	virtual void Render() override;

	Pipeline pipeline;
	RenderMesh3D m_mesh;

};