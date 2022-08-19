#pragma once

#include <GL/glew.h>
#include <SDL.h>
#include <glm/glm.hpp>
#include "effect.h"
#include "gpu_types.h"
#include "gpu_buffer.h"
#include "gpu_program.h"

struct ComputeTestEffect : public Effect
{
	// Inherited via Effect
	virtual bool Init() override;
	virtual bool Update(float time) override;
	virtual bool HandleEvent(const SDL_Event* ev) override;
	virtual void Render() override;

	GpuBuffer vbo_rect;
	GpuProgram prg_compute;
	GpuProgram prg_view;
	GLuint tex_w = 512, tex_h = 512;

	GLuint tex0;
	GLuint vao;
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
		prg_compute(),
		prg_view(),
		vao(0xffff),
		tex0(0xffff),
		angle(0.0f),
		u_angle(-1),
		syncObj()
	{}

	~ComputeTestEffect() noexcept;
};