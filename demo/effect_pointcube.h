#pragma once

#include <memory>

#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "effect.h"
#include "gpu_buffer.h"
#include "gpu_program.h"
#include "gpu_texture.h"
#include "gpu_framebuffer.h"

#define KERNEL_BLUR 0
#define KERNEL_BOTTOM_SOBEL 1
#define KERNEL_IDENTITY 2
#define KERNEL_EMBOSS 3
#define KERNEL_LEFT_SOBEL 4
#define KERNEL_OUTLINE 5
#define KERNEL_RIGHT_SOBEL 6
#define KERNEL_SHARPEN 7
#define KERNEL_TOP_SOBEL 8

const float kernels[][9] = {
	{
		0.0625f, 0.125f, 0.0625f,
		0.125f, 0.25f, 0.125f,
		0.0625f, 0.125f, 0.0625f
	},
	{
		-1.0f, -2.0f, -1.0f,
		 0.0f,  0.0f,  0.0f,
		 1.0f,  2.0f,  1.0f
	},
	{
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
	},
	{
		-2.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 1.0f,
		 0.0f,  1.0f, 2.0f
	},
	{
		1.0f, 0.0f, -1.0f,
		2.0f, 0.0f, -2.0f,
		1.0f, 0.0f, -1.0f
	},
	{
		-1.0f, -1.0f, -1.0f,
		-1.0f,  8.0f, -1.0f,
		-1.0f, -1.0f, -1.0f
	},
	{
		-1.0f, 0.0f, 1.0f,
		-2.0f, 0.0f, 2.0f,
		-1.0f, 0.0f, 1.0f
	},
	{
		 0.0f, -1.0f,  0.0f,
		-1.0f,  5.0f, -1.0f,
		 0.0f, -1.0f,  0.0f
	},
	{
		 1.0f,  2.0f,  1.0f,
		 0.0f,  0.0f,  0.0f,
		-1.0f, -2.0f, -1.0f
	}
};

struct PointCubeEffect : public Effect
{
	~PointCubeEffect();

	PointCubeEffect() :
		eyeZ(1200.0f),
		offset_loc(-1),
		VP(),
		P(),
		rotX(),
		rotY(),
		vao_points(0xffff),
		vao_pp(0xffff),
		vao_skybox(0xffff),
		pp_offset(1.0f/1000.0f),
		fbTex(),
		skyTex_(),
		depthTex(),
		rectWMtx(),
		m_fb(),
		vbo_points(eGpuBufferTarget::VERTEX),
		vbo_pp(eGpuBufferTarget::VERTEX),
		vbo_skybox(eGpuBufferTarget::VERTEX) {};

	bool Init() override;
	bool Update(float time) override;
	void Render() override;
	bool HandleEvent(const SDL_Event* ev, float time) override;

	//GLuint vbo, vbo_pp;
	GpuBuffer vbo_points;
	GpuBuffer vbo_pp;
	GpuBuffer vbo_skybox;

	GLuint vao_points;
	GLuint vao_pp;
	GLuint vao_skybox;

	GpuFrameBuffer m_fb;
	GpuTexture2D::Ptr fbTex;
	GpuTexture2D::Ptr depthTex;
	GpuTextureCubeMap skyTex_;

	GLint rectWMtx;

	GLint offset_loc;
	float pp_offset;

	const int NUMPOINTS = 500000;

	struct VertexLayout
	{
		GLfloat x, y, z;
		GLubyte r, g, b, a;
	};

	struct PPLayout
	{
		GLfloat x, y;
		GLfloat s, t;
	};

	float rotX, rotY, eyeZ;
	glm::mat4 P;
	glm::mat4 VP;

	GpuProgram prgPoints;
	GpuProgram prgPP;
	GpuProgram prgSkybox;
	GpuProgram prgTextureRect;
};