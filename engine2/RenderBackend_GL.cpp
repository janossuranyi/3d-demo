#include <string>
#include <set>
#include <SDL.h>
#include <GL/glew.h>

#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>

#include "./Engine.h"
#include "./RenderSystem.h"
#include "./RenderBackend.h"
#include "./RenderBackend_GL.h"
#include "./FrameBuffer.h"
#include "./Logger.h"


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

		glcontext.currentCullMode = CULL_NONE;
		glcontext.cullEnabled = false;
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
			glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_TRUE);
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
		clearColor[0] = r;
		clearColor[1] = g;
		clearColor[2] = b;
		clearColor[3] = a;
		GL_CHECK( glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]) );
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
		renderSystem.backend->GetScreenSize(x, y);

		renderSystem.programManager->UniformChanged(UBB_FREQ_LOW_VERT);
		renderSystem.programManager->UniformChanged(UBB_FREQ_LOW_FRAG);

		vec3 lightDir = view->spotLightDir;
		vec3 lightPos = view->lightPos;

		mat4 lightProj = perspective(view->renderView.fov, 1.0f, view->nearClipDistance, view->farClipDistance);
		mat4 lightView = lookAt(lightPos, lightPos + lightDir, { 0.0f,1.0f,0.0f });
		mat4 lightViewProj = lightProj * lightView;

		auto& slowvert = renderSystem.programManager->g_freqLowVert;
		auto& slowfrag = renderSystem.programManager->g_freqLowFrag;

		slowvert.viewMatrix = view->renderView.viewMatrix;
		slowvert.projectMatrix = view->projectionMatrix;
		slowvert.lightProjMatrix = lightViewProj;

		slowfrag.nearFarClip = { view->nearClipDistance,view->farClipDistance,0.f,0.f };
		slowfrag.oneOverScreenSize = { 1.0f / (float)x, 1.0f / (float)y,0.f,0.f };
		slowfrag.shadowparams = { 1.0f / (float)renderGlobals.shadowResolution,renderGlobals.shadowScale,renderGlobals.shadowBias,0.0f };
		slowfrag.params.x = view->exposure;
		slowfrag.lightOrig = view->lightPos;
		slowfrag.lightColor = view->lightColor;
		slowfrag.spotLightParams = view->spotLightParams;
		slowfrag.lightAttenuation = view->lightAttenuation;
		slowfrag.spotDirection = view->spotLightDir;
		slowfrag.viewOrigin = vec4(view->renderView.vieworg, 1.f);

		uboUniforms_t& uniforms = renderSystem.programManager->uniforms;

		uniforms.viewOrigin = vec4(view->renderView.vieworg, 1.f);
		uniforms.gExposure = view->exposure;
		uniforms.lightOrig = view->lightPos;
		uniforms.lightColor = view->lightColor;
		uniforms.spotLightParams = view->spotLightParams;
		uniforms.lightAttenuation = view->lightAttenuation;
		uniforms.spotDirection = view->spotLightDir;
		uniforms.nearFarClip.x = view->nearClipDistance;
		uniforms.nearFarClip.y = view->farClipDistance;
		uniforms.gOneOverShadowRes = 1.0f / renderGlobals.shadowResolution;
		uniforms.gShadowScale = renderGlobals.shadowScale;
		uniforms.gShadowBias = renderGlobals.shadowBias;

		RenderShadow();
		RenderDepthPass();
		RenderDebugPass();


		//[...]

	}
	void RenderBackend::RenderCommandBuffer(const emptyCommand_t* cmds)
	{
		renderSystem.vertexCache->Frame();
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
		if (mode != glcontext.currentCullMode)
		{
			glcontext.currentCullMode = mode;
			if (mode == CULL_NONE && glcontext.cullEnabled)
			{
				glDisable(GL_CULL_FACE);
				glcontext.cullEnabled = false;
			}
			else if (mode != CULL_NONE && !glcontext.cullEnabled)
			{
				glEnable(GL_CULL_FACE);
				glcontext.cullEnabled = true;
			}

			switch (mode)
			{
			case CULL_FRONT:
				glCullFace(GL_FRONT);
				break;
			case CULL_BACK:
				glCullFace(GL_BACK);
				break;
			}
		}
	}

	void R_DrawSurf(const drawSurf_t* surf)
	{
		// setup vertex/index buffers
		renderSystem.vertexCache->BindVertexBuffer(surf->vertexCache, 0, sizeof(drawVert_t));
		renderSystem.vertexCache->BindIndexBuffer(surf->indexCache);

		renderSystem.programManager->UpdateUniforms();

		IndexBuffer idx;
		renderSystem.vertexCache->GetIndexBuffer(surf->indexCache, idx);
		GL_CHECK(glDrawElements(
			GL_map_topology(surf->frontEndGeo->topology),
			surf->numIndex,
			GL_UNSIGNED_SHORT,
			(void*)idx.GetOffset()));
		
		perfCounters.drawElements++;
		perfCounters.drawIndexes += surf->numIndex;
	}

	void RenderBackend::RenderDebugPass()
	{
		using namespace glm;

		glDepthMask(GL_FALSE);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		Clear(true, false, false);

		const drawSurf_t* surf;
		SetCurrentTextureUnit(IMU_SHADOW);
		renderSystem.imageManager->globalImages.Depth32->Bind();

		vec3 lightDir = view->spotLightDir;
		vec3 lightPos = view->lightPos;

		mat4 lightProj = perspective(view->renderView.fov, 1.0f, view->nearClipDistance, view->farClipDistance);
		mat4 lightView = lookAt(lightPos, lightPos + lightDir, { 0.0f,1.0f,0.0f });
		mat4 lightViewProj = lightProj * lightView;

		uboUniforms_t& uniforms = renderSystem.programManager->uniforms;
		uniforms.lightProjMatrix = lightViewProj;

		for (int pass = 0; pass < 2; ++pass)
		{
			for (int i = 0; i < view->numDrawSurfs; ++i)
			{
				surf = view->drawSurfs[i];
				const Material* shader = surf->shader;
				if (!shader || shader->IsEmpty()) continue;
				if (shader->GetStage(STAGE_DEBUG).enabled == false) continue;
				const stage_t& stage = shader->GetStage(STAGE_DEBUG);

				if (stage.coverage != COVERAGE_SOLID && stage.coverage != COVERAGE_MASK) continue;
				if (pass == 0 && stage.coverage != COVERAGE_SOLID) continue;
				if (pass == 1 && stage.coverage == COVERAGE_SOLID) continue;

				renderSystem.programManager->UseProgram(stage.shader);

				// setup textures
				for (int j = 0; j < IMU_COUNT; ++j)
				{
					if (stage.images[j])
					{
						renderSystem.backend->SetCurrentTextureUnit(j);
						stage.images[j]->Bind();
					}
				}

				mat4 normalMatrix = transpose(inverse(mat3(surf->space->modelMatrix)));
				uint32 flg_x = (stage.coverage & FLG_X_COVERAGE_MASK) << FLG_X_COVERAGE_SHIFT;

				renderSystem.programManager->UniformChanged(UBB_FREQ_HIGH_VERT);
				renderSystem.programManager->UniformChanged(UBB_FREQ_HIGH_FRAG);
				auto& highvert = renderSystem.programManager->g_freqHighVert;
				auto& highfrag = renderSystem.programManager->g_freqHighFrag;
				highvert.localToWorldMatrix = surf->space->modelMatrix;
				highvert.normalMatrix = normalMatrix;
				highvert.WVPMatrix = surf->space->mvp;
				highfrag.params.x = uintBitsToFloat(flg_x);
				highfrag.matDiffuseFactor = stage.diffuseScale;
				highfrag.matMRFactor.x = stage.roughnessScale;
				highfrag.matMRFactor.y = stage.metallicScale;

				auto& uniforms = renderSystem.programManager->uniforms;
				uniforms.alphaCutoff.x = stage.alphaCutoff;
				uniforms.localToWorldMatrix = surf->space->modelMatrix;
				uniforms.matDiffuseFactor = stage.diffuseScale;
				uniforms.matMRFactor.x = stage.roughnessScale;
				uniforms.matMRFactor.y = stage.metallicScale;
				uniforms.WVPMatrix = surf->space->mvp;
				uniforms.normalMatrix = normalMatrix;
				uniforms.params.x = uintBitsToFloat(flg_x);
				SetCullMode(stage.cullMode);

				R_DrawSurf(surf);
			}
		}
	}

	void RenderBackend::RenderDepthPass()
	{
		using namespace glm;

		if (view == nullptr) return;

		glViewport(0, 0, view->scissor.w, view->scissor.h);
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

			renderSystem.programManager->UniformChanged(UBB_FREQ_HIGH_VERT);
			auto& highvert = renderSystem.programManager->g_freqHighVert;
			highvert.localToWorldMatrix = surf->space->modelMatrix;
			highvert.WVPMatrix = surf->space->mvp;

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

		vec3 lightDir = view->spotLightDir;
		vec3 lightPos = view->lightPos;

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

			renderSystem.programManager->UniformChanged(UBB_FREQ_HIGH_VERT);
			renderSystem.programManager->g_freqHighVert.WVPMatrix = lightViewProj * surf->space->modelMatrix;

			R_DrawSurf(surf);
		}

		Framebuffer::Unbind();
		glDepthFunc(GL_LEQUAL);

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