#include <string>
#include <set>
#include <SDL.h>
#include <GL/glew.h>

#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>

#include "engine2/Engine.h"
#include "engine2/RenderSystem.h"
#include "engine2/RenderBackend.h"
#include "./RenderBackend_GL.h"
#include "engine2/GLState.h"
#include "engine2/FrameBuffer.h"
#include "engine2/Logger.h"
#include "engine2/ImageManager.h"

void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		Error("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
		abort();
	}
}

namespace jsr {
	void R_DrawSurf(const drawSurf_t* surf);

	const imageFormatInfo_t s_image_formats[] = {
		{GL_ZERO,				GL_ZERO,					GL_ZERO,			GL_ZERO,							true,	0}, // IMF_DEFAULT
		{GL_ALPHA,				GL_ZERO,					GL_ALPHA,			GL_UNSIGNED_BYTE,					true,	1}, // IMF_A8
		{GL_R8,					GL_ZERO,					GL_RED,				GL_UNSIGNED_BYTE,					true,	1}, // IMF_R8
		{GL_R16,				GL_ZERO,					GL_RED,				GL_UNSIGNED_SHORT,					true,	1}, // IMF_R16
		{GL_R16F,				GL_ZERO,					GL_RED,				GL_HALF_FLOAT,						false,	1}, // IMF_R16F
		{GL_R32UI,				GL_ZERO,					GL_RED,				GL_UNSIGNED_INT,					true,	1}, // IMF_R32
		{GL_R32F,				GL_ZERO,					GL_RED,				GL_FLOAT,							false,	1}, // IMF_R32F
		{GL_DEPTH_COMPONENT24,	GL_ZERO,					GL_DEPTH_COMPONENT,	GL_UNSIGNED_INT,					true,	1}, // IMF_D24
		{GL_DEPTH24_STENCIL8,	GL_ZERO,					GL_DEPTH_STENCIL,	GL_UNSIGNED_INT_24_8,				true,	1}, // IMF_D24S8
		{GL_DEPTH_COMPONENT32,	GL_ZERO,					GL_DEPTH_COMPONENT,	GL_UNSIGNED_INT,					true,	1}, // IMF_D32
		{GL_DEPTH_COMPONENT32F,	GL_ZERO,					GL_DEPTH_COMPONENT,	GL_FLOAT,							false,	1}, // IMF_D32F
		{GL_RG8,				GL_ZERO,					GL_RG,				GL_UNSIGNED_BYTE,					true,	2}, // IMF_RG8
		{GL_RG16,				GL_ZERO,					GL_RG,				GL_UNSIGNED_SHORT,					true,	2}, // IMF_RG16
		{GL_RG16F,				GL_ZERO,					GL_RG,				GL_HALF_FLOAT,						false,	2}, // IMF_RG16
		{GL_RGB8,				GL_SRGB8,					GL_RGB,				GL_UNSIGNED_BYTE,					true,	3}, // IMF_RGB
		{GL_RGBA8,				GL_SRGB8_ALPHA8,			GL_RGBA,			GL_UNSIGNED_BYTE,					true,	4}, // IMF_RGBA
		{GL_RGB16F,				GL_ZERO,					GL_RGB,				GL_HALF_FLOAT,						false,	3}, // IMF_RGB16F
		{GL_RGBA16F,			GL_ZERO,					GL_RGBA,			GL_HALF_FLOAT,						false,	4}, // IMF_RGBA16F
		{GL_RGB32F,				GL_ZERO,					GL_RGB,				GL_FLOAT,							false,	3}, // IMF_RGB32F
		{GL_RGBA32F,			GL_ZERO,					GL_RGBA,			GL_FLOAT,							false,	4}, // IMF_RGBA32F
		{GL_R11F_G11F_B10F,		GL_ZERO,					GL_RGB,				GL_UNSIGNED_INT_10F_11F_11F_REV,	false,	3}, // IMF_R11G11B10
		{GL_COMPRESSED_RGB,		GL_COMPRESSED_SRGB,			GL_RGB,				GL_UNSIGNED_BYTE,					true,	3}, // IMF_RGB_COMPRESSED
		{GL_COMPRESSED_RGBA,	GL_COMPRESSED_SRGB_ALPHA,	GL_RGBA,			GL_UNSIGNED_BYTE,					true,	4}, // IMF_RGBA_COMPRESSED

		{GL_COMPRESSED_RGB_S3TC_DXT1_EXT,	GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,		GL_RGB,		GL_UNSIGNED_BYTE, true, 3}, // IMF_RGB_COMPRESSED_DXT1
		{GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,	GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, GL_RGBA,	GL_UNSIGNED_BYTE, true, 4}, // IMF_RGBA_COMPRESSED_DXT1
		{GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,	GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, GL_RGBA,	GL_UNSIGNED_BYTE, true, 4}, // IMF_RGBA_COMPRESSED_DXT3
		{GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,	GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, GL_RGBA,	GL_UNSIGNED_BYTE, true, 4}, // IMF_RGBA_COMPRESSED_DXT5
		{GL_COMPRESSED_RGBA_BPTC_UNORM,		GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM,	GL_RGBA,	GL_UNSIGNED_BYTE, true, 4}  // IMF_RGBA_COMPRESSED_BC7
	};

	using namespace std;

	glconfig_t glconfig;


	static GLuint _vertex_layouts[LAYOUT_COUNT];

	void GLAPIENTRY GL_DebugMessageCallback(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar* message,
		const GLvoid* userParam);

	void R_InitVertexLayoutDefs()
	{
		std::memset(&_vertex_layouts[0], 0, sizeof(_vertex_layouts));

		GL_CHECK(glGenVertexArrays(LAYOUT_COUNT, _vertex_layouts));

		// LAYOUT_DRAW_VERT
		GLuint vao = _vertex_layouts[LAYOUT_DRAW_VERT];
		GL_CHECK(glBindVertexArray(vao));
		GL_CHECK(glVertexAttribBinding(0, 0));
		GL_CHECK(glVertexAttribBinding(1, 0));
		GL_CHECK(glVertexAttribBinding(2, 0));
		GL_CHECK(glVertexAttribBinding(3, 0));
		GL_CHECK(glVertexAttribBinding(4, 0));


		GL_CHECK(glVertexAttribFormat(0, 3, GL_FLOAT, false, offsetof(drawVert_t, xyz)));
		GL_CHECK(glVertexAttribFormat(1, 2, GL_FLOAT, false, offsetof(drawVert_t, uv)));
		GL_CHECK(glVertexAttribFormat(2, 4, GL_UNSIGNED_BYTE, true, offsetof(drawVert_t, normal)));
		GL_CHECK(glVertexAttribFormat(3, 4, GL_UNSIGNED_BYTE, true, offsetof(drawVert_t, tangent)));
		GL_CHECK(glVertexAttribFormat(4, 4, GL_UNSIGNED_BYTE, true, offsetof(drawVert_t, color)));

		glVertexBindingDivisor(0, 0);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);

		glconfig.currentVertexArray = vao;
	}

	bool R_InitGfxAPI()
	{
		if (SDL_Init(SDL_INIT_VIDEO) != 0)
		{			
			Error("SDL Init failed: %s", SDL_GetError());
			return false;
		}

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,		24);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,	SDL_TRUE);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,		8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,		8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,		8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,		8);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,	8);

		if (engineConfig.r_fbsrgb) {
			SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, SDL_TRUE);
		}
		if (engineConfig.r_debug) {
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
		}

		//SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, SDL_TRUE);

		if (engineConfig.r_msaa > 0)
		{
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, engineConfig.r_msaa);
		}

		Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

		if (engineConfig.r_fullscreen == 2)
		{
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
		else if (engineConfig.r_fullscreen == 1)
		{
			flags |= SDL_WINDOW_FULLSCREEN;
		}

		glconfig.resX	= engineConfig.r_resX;
		glconfig.resY	= engineConfig.r_resY;
		glconfig.vsync	= engineConfig.r_vsync;

		Info("SDL_CreateWindow start");
		glconfig.hwnd = SDL_CreateWindow("GfxCoreGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, glconfig.resX, glconfig.resY, flags);
		if (!glconfig.hwnd)
		{
			Error("Cannot create window - %s", SDL_GetError());
			return false;
		}

		Info("SDL_CreateWindow done");

		glconfig.glctx = SDL_GL_CreateContext(glconfig.hwnd);
		if (!glconfig.glctx)
		{
			Error("Cannot create GL context - %s", SDL_GetError());
			return false;
		}

		//SDL_GL_MakeCurrent(windowHandle_, glcontext_);

		Info("SDL_GL_CreateContext done");

		// Initialize GLEW
		glewExperimental = true; // Needed for core profile
		if (glewInit() != GLEW_OK) {
			Error("Cannot initialize GLEW");
			return false;
		}

		Info("Using GLEW %s", glewGetString(GLEW_VERSION));

		SDL_GL_SetSwapInterval(glconfig.vsync);

		std::string renderer = (char*)glGetString(GL_RENDERER);
		std::string version = (char*)glGetString(GL_VERSION);

		Info("OpenGL extensions");
		GLint numExts; glGetIntegerv(GL_NUM_EXTENSIONS, &numExts);
		string exts;

		for (int ext_ = 0; ext_ < numExts; ++ext_)
		{
			const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, ext_);
			//Info("%s", extension);
			glconfig.extensions.emplace(extension);
			exts.append(extension).append(" ");
		}
		
		const auto gl_version = atof(version.c_str());
		glconfig.version = int(gl_version * 100);

		if (glconfig.version < 450)
		{
			Error("GL_VERSION < 4.5");
			return false;
		}

		Info("extensions: %s", exts.c_str());
		SDL_version ver;

		SDL_GetVersion(&ver);

		Info("GL Renderer: %s", renderer.c_str());
		Info("GL Version: %s (%.2f)", version.c_str(), gl_version);
		Info("SDL version: %d.%d patch %d", (int)ver.major, (int)ver.minor, (int)ver.patch);

		int _w, _h;

		SDL_GL_GetDrawableSize(glconfig.hwnd, &_w, &_h);
		glconfig.resX = _w;
		glconfig.resY = _h;

		glViewport(0, 0, _w, _h);
		glScissor(0, 0, _w, _h);
		glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS,			&glconfig.maxArrayTextureLayers);

		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,			&glconfig.maxFragmentTextureImageUnits);
		glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,	&glconfig.maxVertexTextureImageUnits);
		glGetIntegerv(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS,	&glconfig.maxComputeTextureImageUnits);

		glGetIntegerv(GL_MAX_TEXTURE_SIZE,					&glconfig.maxTextureSize);
		glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE,	&glconfig.maxComputeSharedMemorySize);
		glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE,			&glconfig.maxUniformBlockSize);
		glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE,		&glconfig.maxShaderStorageBlockSize);
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS,				&glconfig.maxVertexAttribs);
		glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS,		&glconfig.maxVertexAttribBindings);

		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,	&glconfig.uniformBufferOffsetAligment);
		glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &glconfig.availableVideoMemory);

		glGetError();
		glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI,			&glconfig.availableVideoMemory);
		glGetError();

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		// Setup default GL states
		glEnable(GL_DEPTH_TEST);

		glcontext.rasterizer.currentCullMode = CULL_NONE;
		glcontext.rasterizer.cullEnabled = false;
		glCullFace(GL_BACK);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glDisable(GL_STENCIL_TEST);

		glBlendColor(0.f, 0.f, 0.f, 0.f);
		glBlendFunc(GL_ONE, GL_ZERO);
		glBlendEquation(GL_FUNC_ADD);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		if (engineConfig.r_fbsrgb)
		{
			glEnable(GL_FRAMEBUFFER_SRGB);
		}
		else
		{
			glDisable(GL_FRAMEBUFFER_SRGB);
		}
		glcontext.blendState.alphaDst = BFUNC_ZERO;
		glcontext.blendState.colDst = BFUNC_ZERO;
		glcontext.blendState.alphaSrc = BFUNC_ONE;
		glcontext.blendState.colSrc = BFUNC_ONE;
		glcontext.blendState.colorOp = BOP_ADD;
		glcontext.blendState.alphaOp = BOP_ADD;
		glcontext.blendState.enabled = false;

		// end
		if (engineConfig.r_debug && glconfig.extensions.count("GL_ARB_debug_output"))
		{
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

			glDebugMessageCallbackARB(&GL_DebugMessageCallback, NULL);
#if 1
			glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
			glDebugMessageControlARB(GL_DONT_CARE, GL_DEBUG_TYPE_ERROR_ARB, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			glDebugMessageControlARB(GL_DONT_CARE, GL_DEBUG_TYPE_PERFORMANCE_ARB, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			glDebugMessageControlARB(GL_DONT_CARE, GL_DEBUG_TYPE_PORTABILITY_ARB, GL_DONT_CARE, 0, nullptr, GL_TRUE);
			glDebugMessageControlARB(GL_DONT_CARE, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB, GL_DONT_CARE, 0, nullptr, GL_TRUE);

			glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
			glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
			//glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_TRUE);
#endif
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		R_InitVertexLayoutDefs();

		ImGui_ImplSDL2_InitForOpenGL(glconfig.hwnd, glconfig.glctx);
		ImGui_ImplOpenGL3_Init();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();

		return true;

	}

	void R_ShutdownGfxAPI()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();

		if (glconfig.glctx) {
			SDL_GL_DeleteContext(glconfig.glctx);
			glconfig.glctx = 0;
		}
		if (glconfig.hwnd) {
			SDL_DestroyWindow(glconfig.hwnd);
			glconfig.hwnd = nullptr;
		}
	}

	void RenderBackend::SetClearColor(float r, float g, float b, float a)
	{
		if (
			clearColor[0] != r ||
			clearColor[1] != g ||
			clearColor[2] != b ||
			clearColor[3] != a) 
		{
			clearColor[0] = r;
			clearColor[1] = g;
			clearColor[2] = b;
			clearColor[3] = a;
			GL_CHECK(glClearColor(r, g, b, a));
		}
	}

	void RenderBackend::SetClearColor(glm::vec4 color)
	{
		SetClearColor(color.r, color.g, color.b, color.a);
	}

	void RenderBackend::Clear(bool color, bool depth, bool stencil)
	{
		GLbitfield target{};
		if (color)
		{
			target |= GL_COLOR_BUFFER_BIT;
		}
		if (depth)
		{
			target |= GL_DEPTH_BUFFER_BIT;
		}
		if (stencil)
		{
			target |= GL_STENCIL_BUFFER_BIT;
		}
		if (target)
		{
			glClear(target);
		}
	}

	int RenderBackend::GetUniformBufferAligment() const
	{
		return glconfig.uniformBufferOffsetAligment;
	}


	void RenderBackend::RenderView(viewDef_t* view)
	{
		using namespace glm;
		this->view = view;

		if (!view) return;
		int x, y;
		GetScreenSize(x, y);
		SetClearColor(0.f, 0.f, 0.f, 1.0f);

		/**
		mat4 lightProj = perspective(view->renderView.fov, 1.0f, view->nearClipDistance, view->farClipDistance);
		mat4 lightView = lookAt(lightPos, lightPos + lightDir, { 0.0f,1.0f,0.0f });
		mat4 lightViewProj = lightProj * lightView;
		**/

		renderSystem.programManager->BindUniformBlock(UBB_FREQ_LOW_VERT, view->freqLowVert);
		renderSystem.programManager->BindUniformBlock(UBB_FREQ_LOW_FRAG, view->freqLowFrag);

		RenderDeferred_GBuffer();
		RenderDeferred_Lighting();
		RenderHDRtoLDR();
#if 0
		Framebuffer::Unbind();

		GL_CHECK(glViewport(0, 0, x, y));
		Clear(true, false, false);

		GLsizei HalfWidth = (GLsizei)(x / 2);
		GLsizei HalfHeight = (GLsizei)(y / 2);
		Framebuffer* gbuffer = globalFramebuffers.GBufferFBO;
		Framebuffer* hdrbuffer = globalFramebuffers.hdrFBO;

		gbuffer->BindForReading();
		gbuffer->SetReadBuffer(0);
		gbuffer->BlitColorBuffer(0, 0, x, y,
			0, 0, HalfWidth, HalfHeight);

		gbuffer->SetReadBuffer(1);
		gbuffer->BlitColorBuffer(0, 0, x, y,
			0, HalfHeight, HalfWidth, y);

		gbuffer->SetReadBuffer(2);
		gbuffer->BlitColorBuffer(0, 0, x, y,
			HalfWidth, HalfHeight, x, y);

		hdrbuffer->BindForReading();
		hdrbuffer->SetReadBuffer(0);
		gbuffer->BlitColorBuffer(0, 0, x, y,
			HalfWidth, 0, x, HalfHeight);
#endif		
	}

	void RenderBackend::RenderCommandBuffer(const emptyCommand_t* cmds)
	{
		Framebuffer::Unbind();

		for (const emptyCommand_t* cmd = cmds; cmd != nullptr; cmd = (emptyCommand_t*)cmd->next)
		{

			switch (cmd->command)
			{
			case RC_NOP:
				//Info("Render command NOP");
				break;
			case RC_DRAW_VIEW:
				RenderView(((drawViewCommand_t*)cmd)->view);
				break;
			}
		}


		EndFrame();

	}
	void RenderBackend::SetCullMode(eCullMode mode)
	{
//		if (mode != glcontext.rasterizer.currentCullMode)
		{
			//glcontext.rasterizer.currentCullMode = mode;
			
			if (mode == CULL_BACK)
			{
				GL_State(GLS_CULL_FRONTSIDED);
			}
			else if (mode == CULL_FRONT)
			{
				GL_State(GLS_CULL_BACKSIDED);
			}
			else
			{
				GL_State(GLS_CULL_TWOSIDED);
			}
			/*

			if (mode == CULL_NONE && glcontext.rasterizer.cullEnabled)
			{
				glDisable(GL_CULL_FACE);
				glcontext.rasterizer.cullEnabled = false;
			}
			else if (mode != CULL_NONE && !glcontext.rasterizer.cullEnabled)
			{
				glEnable(GL_CULL_FACE);
				glcontext.rasterizer.cullEnabled = true;
			}

			switch (mode)
			{
			case CULL_FRONT:
				glCullFace(GL_FRONT);
				break;
			case CULL_BACK:
				glCullFace(GL_BACK);
				break;
			}*/
		}
	}

	void R_DrawSurf(const drawSurf_t* surf)
	{
		// setup vertex/index buffers
		renderSystem.vertexCache->BindVertexBuffer(surf->vertexCache, 0, sizeof(drawVert_t));
		renderSystem.vertexCache->BindIndexBuffer(surf->indexCache);

		IndexBuffer idx;
		renderSystem.vertexCache->GetIndexBuffer(surf->indexCache, idx);
		const GLenum mode = surf->frontEndGeo ? GL_map_topology(surf->frontEndGeo->topology) : GL_TRIANGLES;
		GL_CHECK(glDrawElements(
			mode,
			surf->numIndex,
			GL_UNSIGNED_SHORT,
			(void*)idx.GetOffset()));
		
		perfCounters.drawElements++;
		perfCounters.drawIndexes += surf->numIndex;
	}
	void RenderBackend::RenderDepthPass()
	{
		using namespace glm;

		if (view == nullptr) return;

		glViewport(view->viewport.x, view->viewport.y, view->viewport.w, view->viewport.h);
		glDepthMask(GL_TRUE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		Clear(false, true, false);

		const drawSurf_t* surf;

		renderSystem.programManager->UseProgram(PRG_ZPASS);

		for (int i = 0; i < view->numDrawSurfs; ++i)
		{
			surf = view->drawSurfs[i];
			const Material* shader = surf->shader;
			if (!shader || shader->IsEmpty()) continue;
			if (shader->GetStage(STAGE_DEBUG).enabled == false) continue;
			const stage_t& stage = shader->GetStage(STAGE_DEBUG);
			if (stage.coverage != COVERAGE_SOLID) continue;

			SetCullMode(stage.cullMode);

			renderSystem.programManager->BindUniformBlock(UBB_FREQ_HIGH_VERT, surf->space->highFreqVert);

			R_DrawSurf(surf);
		}
	}

	void RenderBackend::RenderShadow()
	{
		using namespace glm;

		globalFramebuffers.shadowFBO->Bind();
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
		glViewport(0, 0, renderGlobals.shadowResolution, renderGlobals.shadowResolution);

		Clear(false, true, false);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glEnable(GL_CULL_FACE);

		vec3 lightDir = { 0,0,-1 };
		vec3 lightPos = { 0,0,0 };

		mat4 lightProj = perspective(view->renderView.fov, 1.0f, view->nearClipDistance, view->farClipDistance);
		mat4 lightView = lookAt(lightPos, lightPos + lightDir, { 0.0f,1.0f,0.0f });
		mat4 lightViewProj = lightProj * lightView;

		const drawSurf_t* surf;

		renderSystem.programManager->UseProgram(PRG_ZPASS);

		SetCullMode(CULL_FRONT);

		for (int i = 0; i < view->numDrawSurfs; ++i)
		{
			surf = view->drawSurfs[i];
			const Material* shader = surf->shader;
			if ( ! shader || shader->IsEmpty() ) continue;
			if ( ! shader->GetStage(STAGE_SHADOW).enabled ) continue;

			const stage_t& stage = shader->GetStage(STAGE_SHADOW);
			if ( stage.coverage != COVERAGE_SOLID ) continue;
			/**
			if (glm::any(glm::greaterThan(stage.emissiveScale, vec4(0.0f)))
				|| stage.images[IMU_EMMISIVE] != renderSystem.imageManager->globalImages.whiteImage) continue;
				**/

			R_DrawSurf(surf);
		}

		Framebuffer::Unbind();
		glDepthFunc(GL_LEQUAL);

	}

	void RenderBackend::RenderDeferred_GBuffer()
	{
		using namespace glm;

		globalFramebuffers.GBufferFBO->Bind();
		renderSystem.programManager->UseProgram(PRG_DEFERRED_GBUFFER_MR);

		GL_State(GLS_DEPTHFUNC_LESS);
	
		//glDepthFunc(GL_LEQUAL);
		//glDepthMask(GL_TRUE);
		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		
		GL_CHECK(glViewport(view->viewport.x, view->viewport.y, view->viewport.w, view->viewport.h));

		Clear(true, true, false);

		vec4 v1(1.0f);
		globalFramebuffers.GBufferFBO->ClearAttachment(3, v1);

		const drawSurf_t* surf;

		const eStageType ACTIVE_STAGE = STAGE_DEBUG;

		for (int k = 0; k < 2; ++k)
		{
			for (int i = 0; i < view->numDrawSurfs; ++i)
			{
				surf = view->drawSurfs[i];
				const Material* shader = surf->shader;
				if (!shader || shader->IsEmpty()) continue;
				if (shader->GetStage(ACTIVE_STAGE).enabled == false) continue;
				const stage_t& stage = shader->GetStage(ACTIVE_STAGE);

				if (k == 0 && stage.coverage != COVERAGE_SOLID) continue;
				if (k == 1 && stage.coverage != COVERAGE_MASK) continue;

				if (glm::any(glm::greaterThan(stage.emissiveScale, vec4(0.0f))) || stage.images[IMU_EMMISIVE] != globalImages.whiteImage) continue;
				
				// setup textures
				for (int j = 0; j < IMU_COUNT; ++j)
				{
					if (stage.images[j])
					{
						renderSystem.backend->SetCurrentTextureUnit(j);
						stage.images[j]->Bind();
					}
				}
				SetCullMode(stage.cullMode);

				renderSystem.programManager->BindUniformBlock(UBB_FREQ_HIGH_VERT, surf->space->highFreqVert);
				renderSystem.programManager->BindUniformBlock(UBB_FREQ_HIGH_FRAG, surf->highFreqFrag[ACTIVE_STAGE]);

				R_DrawSurf(surf);
			}
		}
	}

	void RenderBackend::RenderDeferred_Lighting()
	{
		using namespace glm;

		GL_CHECK(glViewport(view->viewport.x, view->viewport.y, view->viewport.w, view->viewport.h));

		globalFramebuffers.hdrFBO->Bind();
		
		globalFramebuffers.GBufferFBO->BindForReading();
		globalFramebuffers.GBufferFBO->BlitDepthBuffer(0, 0, view->viewport.w, view->viewport.h, 0, 0, view->viewport.w, view->viewport.h);
		

		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		Clear(true, false, false);

		renderSystem.programManager->UseProgram(PRG_DEFERRED_LIGHT);
		SetCurrentTextureUnit(IMU_DIFFUSE);
		globalImages.GBufferAlbedo->Bind();
		SetCurrentTextureUnit(IMU_FRAGPOS);
		globalImages.GBufferFragPos->Bind();
		SetCurrentTextureUnit(IMU_NORMAL);
		globalImages.GBufferNormal->Bind();
		SetCurrentTextureUnit(IMU_AORM);
		globalImages.GBufferSpec->Bind();
		SetCurrentTextureUnit(IMU_SHADOW);
		globalImages.Shadow->Bind();

		/*glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_STENCIL_TEST);*/

		GL_State(GLS_BLENDOP_ADD | GLS_DSTBLEND_ONE | GLS_SRCBLEND_ONE | GLS_DEPTHMASK );

		for (const auto* light = view->viewLights; light != nullptr; light = light->next)
		{
			renderSystem.programManager->BindUniformBlock(UBB_FREQ_HIGH_VERT, light->highFreqVert);
			renderSystem.programManager->BindUniformBlock(UBB_FREQ_HIGH_FRAG, light->highFreqFrag);

			SetCullMode(CULL_NONE);
			Clear(false, false, true);
			// We need the stencil test to be enabled but we want it
			// to succeed always. Only the depth test matters.
			/*
			glStencilFunc(GL_ALWAYS, 0, 0);
			glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
			glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glDepthFunc(GL_LEQUAL);*/

			GL_State(GLS_STENCIL_FUNC_ALWAYS | GLS_BACK_STENCIL_OP_ZFAIL_INCR_WRAP | GLS_STENCIL_OP_ZFAIL_DECR_WRAP| GLS_SEPARATE_STENCIL | GLS_COLORMASK|GLS_ALPHAMASK | GLS_DEPTHFUNC_LESS);

			if (light->type == LIGHT_POINT)
			{
				R_DrawSurf(&unitSphereSurface);
			}
			else if (light->type == LIGHT_SPOT)
			{
				R_DrawSurf(&unitConeSurface);
			}

			/*glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
			glDepthFunc(GL_ALWAYS);
			SetCullMode(CULL_FRONT);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);*/

			uint64 stencilParms = 0 << GLS_STENCIL_FUNC_REF_SHIFT | 0xFF << GLS_STENCIL_FUNC_MASK_SHIFT;

			GL_State(GLS_STENCIL_FUNC_NOTEQUAL | stencilParms | GLS_DEPTHFUNC_ALWAYS);

			if (light->type == LIGHT_POINT)
			{
				R_DrawSurf(&unitSphereSurface);
			}
			else if (light->type == LIGHT_SPOT)
			{
				R_DrawSurf(&unitConeSurface);
			}
		}
		
		SetCullMode(CULL_BACK);

	}

	void RenderBackend::RenderHDRtoLDR()
	{
		using namespace glm;

		int w, h;
		GetScreenSize(w, h);
		glViewport(0, 0, w, h);
		/*
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_ALWAYS);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		SetCullMode(CULL_NONE);
		glBlendFunc(GL_ONE, GL_ZERO);
		*/
		GL_State(GLS_DEPTHMASK | GLS_DEPTHFUNC_ALWAYS | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO);

		Framebuffer::Unbind();
		SetCurrentTextureUnit(IMU_HDR);
		globalImages.HDRaccum->Bind();
		SetCurrentTextureUnit(IMU_DIFFUSE);
		globalImages.GBufferAlbedo->Bind();
		renderSystem.programManager->UseProgram(PRG_PP_HDR);
		R_DrawSurf(&unitRectSurface);
	}

	void RenderBackend::RenderDebugPass()
	{
		using namespace glm;

	}

	void RenderBackend::EndFrame()
	{
		// Render dear imgui into screen
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();

		glFinish();
		SDL_GL_SwapWindow(glconfig.hwnd);
	}

	void RenderBackend::GetScreenSize(int& x, int& y) const
	{
		x = glconfig.resX;
		y = glconfig.resY;
	}
	void RenderBackend::SetCurrentTextureUnit(int unit)
	{
		if (unit >= MAX_TEXTURE_UNITS) return;

		currenttmu = unit;
	}

	/*
	====================
	idRenderBackend::GL_State

	This routine is responsible for setting the most commonly changed state
	====================
	*/
	void RenderBackend::GL_State(uint64 stateBits, bool forceGlState)
	{
		uint64 diff = stateBits ^ glStateBits;

		if (!engineConfig.r_useStateCaching || forceGlState)
		{
			// make sure everything is set all the time, so we
			// can see if our delta checking is screwing up
			diff = 0xFFFFFFFFFFFFFFFF;
		}
		else if (diff == 0)
		{
			return;
		}

		//
		// culling
		//
		if (diff & (GLS_CULL_BITS))//| GLS_MIRROR_VIEW ) )
		{
			switch (stateBits & GLS_CULL_BITS)
			{
			case GLS_CULL_TWOSIDED:
				glDisable(GL_CULL_FACE);
				break;

			case GLS_CULL_FRONTSIDED:
				glEnable(GL_CULL_FACE);
				/*if (viewDef != NULL && viewDef->isMirror)
				{
					stateBits |= GLS_MIRROR_VIEW;
					glCullFace(GL_FRONT);
				}
				else*/
				{
					glCullFace(GL_BACK);
				}
				break;

			case GLS_CULL_BACKSIDED:
			default:
				glEnable(GL_CULL_FACE);
				/*if (viewDef != NULL && viewDef->isMirror)
				{
					stateBits |= GLS_MIRROR_VIEW;
					glCullFace(GL_BACK);
				}
				else*/
				{
					glCullFace(GL_FRONT);
				}
				break;
			}
		}

		//
		// check depthFunc bits
		//
		if (diff & GLS_DEPTHFUNC_BITS)
		{
			switch (stateBits & GLS_DEPTHFUNC_BITS)
			{
			case GLS_DEPTHFUNC_EQUAL:
				glDepthFunc(GL_EQUAL);
				break;
			case GLS_DEPTHFUNC_ALWAYS:
				glDepthFunc(GL_ALWAYS);
				break;
			case GLS_DEPTHFUNC_LESS:
				glDepthFunc(GL_LEQUAL);
				break;
			case GLS_DEPTHFUNC_GREATER:
				glDepthFunc(GL_GEQUAL);
				break;
			}
		}

		//
		// check blend bits
		//
		if (diff & (GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS))
		{
			GLenum srcFactor = GL_ONE;
			GLenum dstFactor = GL_ZERO;

			switch (stateBits & GLS_SRCBLEND_BITS)
			{
			case GLS_SRCBLEND_ZERO:
				srcFactor = GL_ZERO;
				break;
			case GLS_SRCBLEND_ONE:
				srcFactor = GL_ONE;
				break;
			case GLS_SRCBLEND_DST_COLOR:
				srcFactor = GL_DST_COLOR;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:
				srcFactor = GL_ONE_MINUS_DST_COLOR;
				break;
			case GLS_SRCBLEND_SRC_ALPHA:
				srcFactor = GL_SRC_ALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
				srcFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case GLS_SRCBLEND_DST_ALPHA:
				srcFactor = GL_DST_ALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
				srcFactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			default:
				assert(!"GL_State: invalid src blend state bits\n");
				break;
			}

			switch (stateBits & GLS_DSTBLEND_BITS)
			{
			case GLS_DSTBLEND_ZERO:
				dstFactor = GL_ZERO;
				break;
			case GLS_DSTBLEND_ONE:
				dstFactor = GL_ONE;
				break;
			case GLS_DSTBLEND_SRC_COLOR:
				dstFactor = GL_SRC_COLOR;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:
				dstFactor = GL_ONE_MINUS_SRC_COLOR;
				break;
			case GLS_DSTBLEND_SRC_ALPHA:
				dstFactor = GL_SRC_ALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:
				dstFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case GLS_DSTBLEND_DST_ALPHA:
				dstFactor = GL_DST_ALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:
				dstFactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			default:
				assert(!"GL_State: invalid dst blend state bits\n");
				break;
			}

			// Only actually update GL's blend func if blending is enabled.
			if (srcFactor == GL_ONE && dstFactor == GL_ZERO)
			{
				glDisable(GL_BLEND);
			}
			else
			{
				glEnable(GL_BLEND);
				glBlendFunc(srcFactor, dstFactor);
			}
		}

		//
		// check depthmask
		//
		if (diff & GLS_DEPTHMASK)
		{
			if (stateBits & GLS_DEPTHMASK)
			{
				glDepthMask(GL_FALSE);
			}
			else
			{
				glDepthMask(GL_TRUE);
			}
		}

		//
		// check colormask
		//
		if (diff & (GLS_REDMASK | GLS_GREENMASK | GLS_BLUEMASK | GLS_ALPHAMASK))
		{
			GLboolean r = (stateBits & GLS_REDMASK) ? GL_FALSE : GL_TRUE;
			GLboolean g = (stateBits & GLS_GREENMASK) ? GL_FALSE : GL_TRUE;
			GLboolean b = (stateBits & GLS_BLUEMASK) ? GL_FALSE : GL_TRUE;
			GLboolean a = (stateBits & GLS_ALPHAMASK) ? GL_FALSE : GL_TRUE;
			glColorMask(r, g, b, a);
		}

		//
		// fill/line mode
		//
		if (diff & GLS_POLYMODE_LINE)
		{
			if (stateBits & GLS_POLYMODE_LINE)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}

		//
		// polygon offset
		//
		if (diff & GLS_POLYGON_OFFSET)
		{
			if (stateBits & GLS_POLYGON_OFFSET)
			{
				glPolygonOffset(polyOfsScale, polyOfsBias);
				glEnable(GL_POLYGON_OFFSET_FILL);
				glEnable(GL_POLYGON_OFFSET_LINE);
			}
			else
			{
				glDisable(GL_POLYGON_OFFSET_FILL);
				glDisable(GL_POLYGON_OFFSET_LINE);
			}
		}

		//
		// stencil
		//
		if (diff & (GLS_STENCIL_FUNC_BITS | GLS_STENCIL_OP_BITS))
		{
			if ((stateBits & (GLS_STENCIL_FUNC_BITS | GLS_STENCIL_OP_BITS)) != 0)
			{
				glEnable(GL_STENCIL_TEST);
			}
			else
			{
				glDisable(GL_STENCIL_TEST);
			}
		}
		if (diff & (GLS_STENCIL_FUNC_BITS | GLS_STENCIL_FUNC_REF_BITS | GLS_STENCIL_FUNC_MASK_BITS))
		{
			GLuint ref = GLuint((stateBits & GLS_STENCIL_FUNC_REF_BITS) >> GLS_STENCIL_FUNC_REF_SHIFT);
			GLuint mask = GLuint((stateBits & GLS_STENCIL_FUNC_MASK_BITS) >> GLS_STENCIL_FUNC_MASK_SHIFT);
			GLenum func = 0;

			switch (stateBits & GLS_STENCIL_FUNC_BITS)
			{
			case GLS_STENCIL_FUNC_NEVER:
				func = GL_NEVER;
				break;
			case GLS_STENCIL_FUNC_LESS:
				func = GL_LESS;
				break;
			case GLS_STENCIL_FUNC_EQUAL:
				func = GL_EQUAL;
				break;
			case GLS_STENCIL_FUNC_LEQUAL:
				func = GL_LEQUAL;
				break;
			case GLS_STENCIL_FUNC_GREATER:
				func = GL_GREATER;
				break;
			case GLS_STENCIL_FUNC_NOTEQUAL:
				func = GL_NOTEQUAL;
				break;
			case GLS_STENCIL_FUNC_GEQUAL:
				func = GL_GEQUAL;
				break;
			case GLS_STENCIL_FUNC_ALWAYS:
				func = GL_ALWAYS;
				break;
			}
			glStencilFunc(func, ref, mask);
		}
		if (diff & (GLS_STENCIL_OP_FAIL_BITS | GLS_STENCIL_OP_ZFAIL_BITS | GLS_STENCIL_OP_PASS_BITS))
		{
			GLenum sFail = 0;
			GLenum zFail = 0;
			GLenum pass = 0;

			switch (stateBits & GLS_STENCIL_OP_FAIL_BITS)
			{
			case GLS_STENCIL_OP_FAIL_KEEP:
				sFail = GL_KEEP;
				break;
			case GLS_STENCIL_OP_FAIL_ZERO:
				sFail = GL_ZERO;
				break;
			case GLS_STENCIL_OP_FAIL_REPLACE:
				sFail = GL_REPLACE;
				break;
			case GLS_STENCIL_OP_FAIL_INCR:
				sFail = GL_INCR;
				break;
			case GLS_STENCIL_OP_FAIL_DECR:
				sFail = GL_DECR;
				break;
			case GLS_STENCIL_OP_FAIL_INVERT:
				sFail = GL_INVERT;
				break;
			case GLS_STENCIL_OP_FAIL_INCR_WRAP:
				sFail = GL_INCR_WRAP;
				break;
			case GLS_STENCIL_OP_FAIL_DECR_WRAP:
				sFail = GL_DECR_WRAP;
				break;
			}
			switch (stateBits & GLS_STENCIL_OP_ZFAIL_BITS)
			{
			case GLS_STENCIL_OP_ZFAIL_KEEP:
				zFail = GL_KEEP;
				break;
			case GLS_STENCIL_OP_ZFAIL_ZERO:
				zFail = GL_ZERO;
				break;
			case GLS_STENCIL_OP_ZFAIL_REPLACE:
				zFail = GL_REPLACE;
				break;
			case GLS_STENCIL_OP_ZFAIL_INCR:
				zFail = GL_INCR;
				break;
			case GLS_STENCIL_OP_ZFAIL_DECR:
				zFail = GL_DECR;
				break;
			case GLS_STENCIL_OP_ZFAIL_INVERT:
				zFail = GL_INVERT;
				break;
			case GLS_STENCIL_OP_ZFAIL_INCR_WRAP:
				zFail = GL_INCR_WRAP;
				break;
			case GLS_STENCIL_OP_ZFAIL_DECR_WRAP:
				zFail = GL_DECR_WRAP;
				break;
			}
			switch (stateBits & GLS_STENCIL_OP_PASS_BITS)
			{
			case GLS_STENCIL_OP_PASS_KEEP:
				pass = GL_KEEP;
				break;
			case GLS_STENCIL_OP_PASS_ZERO:
				pass = GL_ZERO;
				break;
			case GLS_STENCIL_OP_PASS_REPLACE:
				pass = GL_REPLACE;
				break;
			case GLS_STENCIL_OP_PASS_INCR:
				pass = GL_INCR;
				break;
			case GLS_STENCIL_OP_PASS_DECR:
				pass = GL_DECR;
				break;
			case GLS_STENCIL_OP_PASS_INVERT:
				pass = GL_INVERT;
				break;
			case GLS_STENCIL_OP_PASS_INCR_WRAP:
				pass = GL_INCR_WRAP;
				break;
			case GLS_STENCIL_OP_PASS_DECR_WRAP:
				pass = GL_DECR_WRAP;
				break;
			}
			glStencilOp(sFail, zFail, pass);
		}

		glStateBits = stateBits;
	}

	static void GLAPIENTRY GL_DebugMessageCallback(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar* message,
		const GLvoid* userParam)
	{
		// Convert enums into a humen readable text
		// See: https://www.opengl.org/registry/specs/ARB/debug_output.txt

	//    if (severity > GL_DEBUG_SEVERITY_MEDIUM) return;

		const char* sourceText = "Unknown";
		switch (source)
		{
		case GL_DEBUG_SOURCE_API_ARB:
			// The GL
			sourceText = "API";
			break;
			// The GLSL shader compiler or compilers for other extension - provided languages
		case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
			sourceText = "Shader compiler";
			break;
			// The window system, such as WGL or GLX
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
			sourceText = "Window system";
			break;
			// External debuggers or third-party middleware libraries
		case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
			sourceText = "Third party";
			break;
			// The application
		case GL_DEBUG_SOURCE_APPLICATION_ARB:
			sourceText = "Application";
			break;
			// Sources that do not fit to any of the ones listed above
		case GL_DEBUG_SOURCE_OTHER_ARB:
			sourceText = "Other";
			break;
		}

		const char* typeText = "Unknown";
		switch (type)
		{
			// Events that generated an error
		case GL_DEBUG_TYPE_ERROR_ARB:
			typeText = "Error";
			break;
			// Behavior that has been marked for deprecation
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
			typeText = "Deprecated behavior";
			break;
			// Behavior that is undefined according to the specification
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
			typeText = "Undefined behavior";
			break;
			// Implementation-dependent performance warnings
		case GL_DEBUG_TYPE_PERFORMANCE_ARB:
			typeText = "Performance";
			break;
			// Use of extensions or shaders in a way that is highly vendor - specific
		case GL_DEBUG_TYPE_PORTABILITY_ARB:
			typeText = "Portability";
			break;
			// Types of events that do not fit any of the ones listed above
		case GL_DEBUG_TYPE_OTHER_ARB:
			typeText = "Other";
			break;
		}

		const char* severityText = "Unknown";
		switch (severity)
		{
			// Any GL error; dangerous undefined behavior; any GLSL or ARB shader compiler and linker errors;
		case GL_DEBUG_SEVERITY_HIGH_ARB:
			severityText = "High";
			break;
			// Severe performance warnings; GLSL or other shader compiler and linker warnings; use of currently deprecated behavior
		case GL_DEBUG_SEVERITY_MEDIUM_ARB:
			severityText = "Medium";
			break;
			// Performance warnings from redundant state changes; trivial undefined behavior
		case GL_DEBUG_SEVERITY_LOW_ARB:
			severityText = "Low";
			break;
		}

		// Unused params
		(void)id;
		(void)length;
		(void)userParam;

		// Replace LogDebug with your logging function
		Info("[OpenGL:source='%s', type='%s', severity='%s'] %s",
			sourceText,
			typeText,
			severityText,
			message);
	}

		GLenum GL_map_topology(eTopology x)
		{
			switch (x)
			{
			case TP_POINTS:	return GL_POINTS;
			case TP_LINES: return GL_LINES;
			case TP_LINE_STRIPS: return GL_LINE_STRIP;
			case TP_TRIANGLES: return GL_TRIANGLES;
			case TP_TRIANGLE_FANS: return GL_TRIANGLE_FAN;
			case TP_TRIANGLE_STRIPS: return GL_TRIANGLE_STRIP;
			default: 
				assert(false);
			}
		}

		GLenum GL_map_texfilter(eImageFilter x)
		{
			switch (x)
			{
			case IFL_NEAREST:			return GL_NEAREST;
			case IFL_LINEAR:			return GL_LINEAR;
			case IFL_NEAREST_NEAREST:	return GL_NEAREST_MIPMAP_NEAREST;
			case IFL_NEAREST_LINEAR:	return GL_NEAREST_MIPMAP_LINEAR;
			case IFL_LINEAR_NEAREST:	return GL_LINEAR_MIPMAP_NEAREST;
			case IFL_LINEAR_LINEAR:		return GL_LINEAR_MIPMAP_LINEAR;
			default:
				assert(false);
			}
		}

		GLenum GL_map_texrepeat(eImageRepeat x)
		{
			switch (x)
			{
			case IMR_REPEAT:			return GL_REPEAT;
			case IMR_CLAMP_TO_BORDER:	return GL_CLAMP_TO_BORDER;
			case IMR_CLAMP_TO_EDGE:		return GL_CLAMP_TO_EDGE;
			default:
				assert(false);
			}
		}
		GLenum GL_map_textarget(eImageShape x)
		{
			switch (x)
			{
			case IMS_2D:			return GL_TEXTURE_2D;
			case IMS_2D_ARRAY:		return GL_TEXTURE_2D_ARRAY;
			case IMS_CUBEMAP:		return GL_TEXTURE_CUBE_MAP;
			case IMS_CUBEMAP_ARRAY:	return GL_TEXTURE_CUBE_MAP_ARRAY;
			default:
				assert(false);
			}
		}
}