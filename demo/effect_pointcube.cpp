#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <vector>

#include "effect_pointcube.h"
#include "demo.h"
#include "logger.h"
#include "filesystem.h"
#include "gpu_buffer.h"
#include "stb_image.h"
#include "gpu_types.h"
#include "gpu_utils.h"
#include "gpu_texture.h"
#include "unit_rect.h"
#include "unit_box.h"

//#define USE_RENDERBUFFER_Z

GLsizei FB_X, FB_Y;


PointCubeEffect::~PointCubeEffect()
{
	GL_CHECK(glUseProgram(0));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	if (vao_points != 0xffff) glDeleteVertexArrays(1, &vao_points);
	if (vao_pp != 0xffff) glDeleteVertexArrays(1, &vao_pp);
	if (vao_skybox != 0xffff) glDeleteVertexArrays(1, &vao_skybox);
	if (fbo != 0xffff) glDeleteFramebuffers(1, &fbo);
	if (fbTex != 0xffff) glDeleteTextures(1, &fbTex);
	if (depthTex != 0xffff) glDeleteTextures(1, &depthTex);
	if (skyTex != 0xffff) glDeleteTextures(1, &skyTex);
	if (rboZ != 0xffff) glDeleteRenderbuffers(1, &rboZ);
}

bool PointCubeEffect::Init()
{
	/*
	create our own framebuffer
	*/

	// 2xSSAA
	FB_X = videoConf.width;
	FB_Y = videoConf.height;
	//FB_X = 1024*2;
	//FB_Y = 1024*2;

	while (glGetError() != GL_NO_ERROR) {}

	// skybox cube map
	GL_CHECK(glGenTextures(1, &skyTex));
	GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, skyTex));

	std::vector<std::string> textures_faces = {
		"assets/textures/skybox/right.jpg",
		"assets/textures/skybox/left.jpg",
		"assets/textures/skybox/top.jpg",
		"assets/textures/skybox/bottom.jpg",
		"assets/textures/skybox/front.jpg",
		"assets/textures/skybox/back.jpg",
	};

	GpuTexture2D t2d;
	t2d
		.withMinFilter(eTexMinFilter::LINEAR)
		.withMagFilter(eTexMagFilter::LINEAR)
		.withWrapS(eTexWrap::CLAMP_TO_EDGE)
		.withWrapT(eTexWrap::CLAMP_TO_BORDER)
		.updateParameters();

	GpuTextureCubeMap tc;
	tc
		.withMinFilter(eTexMinFilter::LINEAR)
		.withMagFilter(eTexMagFilter::LINEAR)
		.withWrapS(eTexWrap::CLAMP_TO_EDGE)
		.withWrapT(eTexWrap::CLAMP_TO_BORDER)
		.withWrapR(eTexWrap::CLAMP_TO_BORDER)
		.updateParameters();

	int width, height, nrChannels;
	unsigned char* data;
	for (unsigned int i = 0; i < textures_faces.size(); i++)
	{
		const std::string filename = g_fileSystem.resolve(textures_faces[i]);
		data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
		if (!data)
		{
			Error("Cannot load texture %s", filename);
			return false;
		}

		GLenum tFormat = nrChannels == 4 ? GL_RGBA : GL_RGB;
		GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_COMPRESSED_SRGB, width, height, 0, tFormat, GL_UNSIGNED_BYTE, data));
		stbi_image_free(data);
	}

	GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));


	GL_CHECK(glGenTextures(1, &fbTex));
	GL_CHECK(glGenFramebuffers(1, &fbo));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, fbTex));
	GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FB_X, FB_Y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

	GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbTex, 0));

#ifdef USE_RENDERBUFFER_Z
	GL_CHECK(glGenRenderbuffers(1, &rboZ));
	GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, rboZ));
	GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, FB_X, FB_Y));
	GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, 0));

	GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboZ));
#else
	GL_CHECK(glGenTextures(1, &depthTex));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, depthTex));
	GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, FB_X, FB_Y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

	GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0));
#endif

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		Error("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
		return false;
	}

	for (int i = 0; i < 15; ++i)
	{
		GL_CHECK(glDisableVertexAttribArray(i));
	}

	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	GL_CHECK(glCreateVertexArrays(1, &vao_points));
	GL_CHECK(glCreateVertexArrays(1, &vao_pp));
	GL_CHECK(glCreateVertexArrays(1, &vao_skybox));
	

	const GLsizeiptr bufSize = sizeof(VertexLayout) * NUMPOINTS;
	vbo_points.create(bufSize, eGpuBufferUsage::STATIC);

	uint8_t *ptr = vbo_points.map(eGpuBufferAccess::MAP_WRITEONLY);
	VertexLayout* buffer = reinterpret_cast<VertexLayout*>(ptr);

	Info("VertexBuffer: allocated %d bytes, mapped at %p", (int)bufSize, buffer);

	if (!ptr) return false;

	const int n = 1000, n2 = n / 2; // particles spread in the cube

	for (unsigned i = 0; i < NUMPOINTS; ++i)
	{
		const float x = static_cast<float>(rand() % n - n2);
		const float y = static_cast<float>(rand() % n - n2);
		const float z = static_cast<float>(rand() % n - n2);
		buffer[i].x = x;
		buffer[i].y = y;
		buffer[i].z = z;
		buffer[i].r = static_cast<GLubyte>( 255 * ((x / n) + 0.5f) );
		buffer[i].g = static_cast<GLubyte>( 255 * ((y / n) + 0.5f) );
		buffer[i].b = static_cast<GLubyte>( 255 * ((z / n) + 0.5f) );
		buffer[i].a = 255;
	}
	vbo_points.unMap();


	vbo_pp.create(6 * sizeof(PPLayout), eGpuBufferUsage::STATIC, UNIT_RECT_WITH_ST);

	vbo_skybox.create(sizeof(UNIT_BOX_POSITIONS), eGpuBufferUsage::STATIC, UNIT_BOX_POSITIONS);

	if (!prgPoints.loadShader(g_fileSystem.resolve("assets/shaders/draw_point.vs.glsl"), g_fileSystem.resolve("assets/shaders/draw_point.fs.glsl")))
	{
		Error("Cannot load shader 'draw_point'");
		return false;
	}

	prgPoints.mapLocationToIndex("m_WVP", 0);

	if (!prgPP.loadShader(g_fileSystem.resolve("assets/shaders/kernel.vs.glsl"), g_fileSystem.resolve("assets/shaders/kernel.fs.glsl")))
	{
		Error("Cannot load shader 'kernel'");
		return false;
	}


	prgPP.mapLocationToIndex("samp0", 0);
	prgPP.mapLocationToIndex("g_kernel", 1);
	prgPP.mapLocationToIndex("g_offset", 2);

	prgPP.use();
	prgPP.set(2, pp_offset);
	prgPP.set(1, 9, kernels[KERNEL_BLUR]);
	GL_CHECK(glUseProgram(0));

	if (!prgSkybox.loadShader(g_fileSystem.resolve("assets/shaders/skybox.vs.glsl"), g_fileSystem.resolve("assets/shaders/skybox.fs.glsl")))
	{
		Error("Cannot load shader 'skybox'");
		return false;
	}

	prgSkybox.use();
	prgSkybox.mapLocationToIndex("m_V", 0);
	prgSkybox.mapLocationToIndex("m_P", 1);
	prgSkybox.mapLocationToIndex("samp0", 2);

	prgSkybox.set(2, 0);

	glm::vec3 viewPos{ 0, 0, eyeZ };

	glm::mat4 V = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	P = glm::perspective(45.0f, (float)videoConf.width / videoConf.height, 1.0f, 1700.0f);

	VP = P * V;

	glm::mat4 sky_view = glm::mat4(glm::mat3(V));
	prgSkybox.set(0, false, sky_view);
	prgSkybox.set(1, false, P);

	GL_CHECK(glUseProgram(0));

	if (!prgTextureRect.loadShader(g_fileSystem.resolve("assets/shaders/view_depthbuf.vs.glsl"), g_fileSystem.resolve("assets/shaders/view_depthbuf.fs.glsl")))
	{
		Error("Cannot load shader 'texture_rect'");
		return false;
	}

	prgTextureRect.use();
	prgTextureRect.mapLocationToIndex("samp0", 0);
	prgTextureRect.mapLocationToIndex("m_W", 1);
	prgTextureRect.set(0, 0);
	
	
	glm::mat4 rectTrans(1.0);
	rectTrans = glm::translate(rectTrans, glm::vec3(0.75f, 0.75f, 0.0f));
	rectTrans = glm::scale(rectTrans, glm::vec3(.25f, .25f, 1.0f));
	prgTextureRect.set(1, false, rectTrans);

	GL_CHECK(glUseProgram(0));

	//glPointSize(2.5f);
	GL_CHECK(glEnable(GL_PROGRAM_POINT_SIZE));
	GL_CHECK(glEnable(GL_DEPTH_TEST));
	GL_CHECK(glDepthFunc(GL_LEQUAL));
	GL_CHECK(glDisable(GL_BLEND));
	GL_CHECK(glDisable(GL_CULL_FACE));

	GL_CHECK(glBlendEquation(GL_FUNC_ADD));
	GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	GL_CHECK(glActiveTexture(GL_TEXTURE0));

	const float lum = 0.1f;
	GL_CHECK(glClearColor(lum, lum*2, lum*3, 1.0f ));


	GL_CHECK(glBindVertexArray(vao_points));
	vbo_points.bind();
	GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexLayout), (void *)0));
	GL_CHECK(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexLayout), (void*)12));
	GL_CHECK(glEnableVertexAttribArray(0));
	GL_CHECK(glEnableVertexAttribArray(1));
	GL_CHECK(glBindVertexArray(0));
	GL_CHECK(glDisableVertexAttribArray(0));
	GL_CHECK(glDisableVertexAttribArray(1));

	GL_CHECK(glBindVertexArray(vao_pp));
	vbo_pp.bind();
	GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(PPLayout), (void *)0));
	GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(PPLayout), (void *)8));
	GL_CHECK(glEnableVertexAttribArray(0));
	GL_CHECK(glEnableVertexAttribArray(1));
	GL_CHECK(glBindVertexArray(0));
	GL_CHECK(glDisableVertexAttribArray(0));
	GL_CHECK(glDisableVertexAttribArray(1));

	GL_CHECK(glBindVertexArray(vao_skybox));
	vbo_skybox.bind();
	GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0));
	GL_CHECK(glEnableVertexAttribArray(0));
	GL_CHECK(glBindVertexArray(0));
	GL_CHECK(glDisableVertexAttribArray(0));

	return true;

}

bool PointCubeEffect::Update(float time)
{
	rotX += time * 0.015f;
	rotY += time * 0.01f;
	//rotX = 15.0f;
	rotX = std::fmodf(rotX, 360.0f);
	rotY = std::fmodf(rotY, 360.0f);

	return true;
}

void PointCubeEffect::Render()
{
	glm::mat4 W(1.0f);
	W = glm::rotate(W, glm::radians(rotX), glm::vec3(1, 0, 0));
	W = glm::rotate(W, glm::radians(rotY), glm::vec3(0, 1, 0));
	
	const glm::mat4 WVP = VP * W;

	glDisable(GL_BLEND);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, FB_X, FB_Y);

	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, 0);


	glBindVertexArray(vao_points);

	prgPoints.use();
	prgPoints.set(0, false, WVP);

	glDrawArrays(GL_POINTS, 0, NUMPOINTS);

	glDepthMask(GL_FALSE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyTex);

	glBindVertexArray(vao_skybox);
	prgSkybox.use();

	const glm::mat4 sky_view = glm::mat4(glm::mat3(W));
	prgSkybox.set(0, false, sky_view);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	glDepthMask(GL_TRUE);
	glViewport(0, 0, videoConf.width, videoConf.height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindVertexArray(vao_pp);

	prgPP.use();
	prgPP.set(2, pp_offset);

	glBindTexture(GL_TEXTURE_2D, fbTex);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_FRAMEBUFFER_SRGB);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisable(GL_FRAMEBUFFER_SRGB);

	//glEnable(GL_BLEND);

	//glViewport(0, 0, 400, 250);

	prgTextureRect.use();
	glBindTexture(GL_TEXTURE_2D, depthTex);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	

}

bool PointCubeEffect::HandleEvent(const SDL_Event* ev)
{
	bool mustUpdate = false;

	if (ev->type == SDL_KEYDOWN)
	{
		switch (ev->key.keysym.sym)
		{
		case SDLK_w:
			eyeZ -= 5.0f;
			mustUpdate = true;
			break;
		case SDLK_s:
			eyeZ += 5.0f;
			mustUpdate = true;
			break;
		case SDLK_x:
			if (pp_offset >= 0.0005) pp_offset -= 0.001;
			break;
		case SDLK_c:
			pp_offset += 0.0005;
			break;
		case SDLK_SPACE:
			return false;
		}
	}

	if (mustUpdate)
	{
		glm::vec3 viewPos{ 0, 0, eyeZ };

		glm::mat4 V = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

		VP = P * V;
	}

	//SDL_Log("ev.type: %d, mouseX: %d, mouseY: %d", ev->type, ev->motion.x, ev->motion.y);

	return true;
}
