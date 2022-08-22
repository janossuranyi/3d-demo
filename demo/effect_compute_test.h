#pragma once

#include <GL/glew.h>
#include <SDL.h>
#include <glm/glm.hpp>
#include "effect.h"
#include "gpu_types.h"
#include "gpu_buffer.h"
#include "gpu_program.h"
#include "gpu_texture.h"
#include "gpu_vertex_layout.h"

struct ComputeTestEffect : public Effect
{
	// Inherited via Effect
	virtual bool Init() override;
	virtual bool Update(float time) override;
	virtual bool HandleEvent(const SDL_Event* ev) override;
	virtual void Render() override;

	struct cbvars_t {
		float angle;
	} *cb_vars;

	GpuBuffer cbo;
	GpuBuffer vbo_rect;
	GpuProgram prg_compute;
	GpuProgram prg_view;
	GLuint tex_w = 512, tex_h = 512;

	GpuTexture2D tex0_;
	VertexLayout layout;
	float angle;
	GLint u_angle;
	GLsync syncObj;

	struct vertexLayout_t
	{
		float x, y;
		float s, t;
	};

	ComputeTestEffect() :
		vbo_rect(eGpuBufferTarget::VERTEX),
		cbo(eGpuBufferTarget::UNIFORM),
		prg_compute(),
		prg_view(),
		angle(0.0f),
		u_angle(-1),
		syncObj(),
		tex0_(),
		layout(),
		cb_vars()
	{}

	~ComputeTestEffect() noexcept;
};