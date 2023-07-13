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

#define JSR_R_SHARED_NUMPARAMS 8

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
		{GL_R16F,				GL_ZERO,					GL_RED,				GL_FLOAT,							false,	1}, // IMF_R16F
		{GL_R32UI,				GL_ZERO,					GL_RED,				GL_UNSIGNED_INT,					true,	1}, // IMF_R32
		{GL_R32F,				GL_ZERO,					GL_RED,				GL_FLOAT,							false,	1}, // IMF_R32F
		{GL_DEPTH_COMPONENT24,	GL_ZERO,					GL_DEPTH_COMPONENT,	GL_UNSIGNED_INT,					true,	1}, // IMF_D24
		{GL_DEPTH24_STENCIL8,	GL_ZERO,					GL_DEPTH_STENCIL,	GL_UNSIGNED_INT_24_8,				true,	1}, // IMF_D24S8
		{GL_DEPTH_COMPONENT32,	GL_ZERO,					GL_DEPTH_COMPONENT,	GL_UNSIGNED_INT,					true,	1}, // IMF_D32
		{GL_DEPTH_COMPONENT32F,	GL_ZERO,					GL_DEPTH_COMPONENT,	GL_FLOAT,							false,	1}, // IMF_D32F
		{GL_RG8,				GL_ZERO,					GL_RG,				GL_UNSIGNED_BYTE,					true,	2}, // IMF_RG8
		{GL_RG16,				GL_ZERO,					GL_RG,				GL_UNSIGNED_SHORT,					true,	2}, // IMF_RG16
		{GL_RG16F,				GL_ZERO,					GL_RG,				GL_FLOAT,							false,	2}, // IMF_RG16
		{GL_RGB8,				GL_SRGB8,					GL_RGB,				GL_UNSIGNED_BYTE,					true,	3}, // IMF_RGB
		{GL_RGBA8,				GL_SRGB8_ALPHA8,			GL_RGBA,			GL_UNSIGNED_BYTE,					true,	4}, // IMF_RGBA
		{GL_RGB16F,				GL_ZERO,					GL_RGB,				GL_FLOAT,							false,	3}, // IMF_RGB16F
		{GL_RGBA16F,			GL_ZERO,					GL_RGBA,			GL_FLOAT,							false,	4}, // IMF_RGBA16F
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

	
	//static void R_DrawFullscreenTri()
	//{
	//	GL_CHECK(glBindVertexBuffer(0, 0, 0, 0));
	//	GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, 3));
	//	glcontext.vtxBindings[0] = { 0,0,0 };
	//}

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
			SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, SDL_FALSE);
		}
		else
		{
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

		int query;
		glGetIntegerv(GL_MIN_PROGRAM_TEXEL_OFFSET, &query);
		Info("GL_MIN_PROGRAM_TEXEL_OFFSET : %d", query);
		glGetIntegerv(GL_MAX_PROGRAM_TEXEL_OFFSET, &query);
		Info("GL_MAX_PROGRAM_TEXEL_OFFSET : %d", query);

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
		glcontext.blendState.opts.alphaDst = BFUNC_ZERO;
		glcontext.blendState.opts.colDst = BFUNC_ZERO;
		glcontext.blendState.opts.alphaSrc = BFUNC_ONE;
		glcontext.blendState.opts.colSrc = BFUNC_ONE;
		glcontext.blendState.opts.colorOp = BOP_ADD;
		glcontext.blendState.opts.alphaOp = BOP_ADD;
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

			//glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
			//glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
			//glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_TRUE);
#endif
		}

		glcontext.sharedUboAlloced = 0;
		glcontext.sharedUboSize = 16 * 1024;
		glcontext.sharedUbo.AllocBufferObject(nullptr, glcontext.sharedUboSize, BU_DYNAMIC, BM_WRITE | BM_PERSISTENT | BM_COHERENT);
		glcontext.sharedUbo.MapBuffer(BM_WRITE | BM_PERSISTENT | BM_COHERENT);

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

	void RenderBackend::SetClearColor(const glm::vec4& color)
	{
		SetClearColor(color.r, color.g, color.b, color.a);
	}

	void RenderBackend::SetWriteMask(const glm::bvec4& mask)
	{
		if (glcontext.writeMask != mask)
		{
			glcontext.writeMask = mask;
			GLboolean r = mask.r ? GL_TRUE : GL_FALSE;
			GLboolean g = mask.g ? GL_TRUE : GL_FALSE;
			GLboolean b = mask.b ? GL_TRUE : GL_FALSE;
			GLboolean a = mask.a ? GL_TRUE : GL_FALSE;
			GL_CHECK(glColorMask(r, g, b, a));
		}
	}

	void RenderBackend::SetDepthState(const depthState_t& state)
	{
		if (state.enabled != glcontext.depthState.enabled)
		{
			glcontext.depthState.enabled = state.enabled;
			if (state.enabled)
			{
				GL_CHECK(glEnable(GL_DEPTH_TEST));
			}
			else
			{
				GL_CHECK(glDisable(GL_DEPTH_TEST));
			}
		}
		if (state.depthMask != glcontext.depthState.depthMask)
		{
			glcontext.depthState.depthMask = state.depthMask;
			if (state.depthMask)
			{
				GL_CHECK(glDepthMask(GL_TRUE));
			}
			else
			{
				GL_CHECK(glDepthMask(GL_FALSE));
			}
		}
		if (state.func != glcontext.depthState.func)
		{
			glcontext.depthState.func = state.func;
			GLenum fn = GL_map_CmpOp(state.func);
			GL_CHECK(glDepthFunc(fn));
		}
	}

	void RenderBackend::SetStencilState(const stencilState_t& state)
	{
		if (glcontext.stencilState == state) return;
		if (glcontext.stencilState.enabled != state.enabled)
		{
			glcontext.stencilState.enabled = state.enabled;
			if (state.enabled)
			{
				GL_CHECK(glEnable(GL_STENCIL_TEST));
			}
			else
			{
				GL_CHECK(glDisable(GL_STENCIL_TEST));
			}
		}
		if (glcontext.stencilState.stencilFunc != state.stencilFunc || glcontext.stencilState.ref != state.ref || glcontext.stencilState.mask != state.mask)
		{
			glcontext.stencilState.stencilFunc = state.stencilFunc;
			glcontext.stencilState.mask = state.mask;
			glcontext.stencilState.ref = state.ref;
			GLenum op = GL_map_CmpOp(state.stencilFunc);
			GL_CHECK(glStencilFunc(op, (GLint)state.ref, (GLuint)state.mask));
		}
		if (
			glcontext.stencilState.fail != state.fail ||
			glcontext.stencilState.zfail != state.zfail ||
			glcontext.stencilState.pass != state.pass ||

			glcontext.stencilState.back_fail != state.back_fail ||
			glcontext.stencilState.back_zfail != state.back_zfail ||
			glcontext.stencilState.back_pass != state.back_pass ||
			glcontext.stencilState.separate != state.separate)
		{
			memcpy(&glcontext.stencilState, &state, sizeof(state));
			GLenum fail{};
			GLenum zfail{};
			GLenum pass{};

			fail = GL_map_stencilOp(state.fail);
			zfail = GL_map_stencilOp(state.zfail);
			pass = GL_map_stencilOp(state.pass);

			if (state.separate)
			{
				GL_CHECK(glStencilOpSeparate(GL_FRONT, fail, zfail, pass));
				fail = GL_map_stencilOp(state.back_fail);
				zfail = GL_map_stencilOp(state.back_zfail);
				pass = GL_map_stencilOp(state.back_pass);
				GL_CHECK(glStencilOpSeparate(GL_BACK, fail, zfail, pass));
			}
			else
			{
				GL_CHECK(glStencilOp(fail, zfail, pass));
			}
		}

	}

	void RenderBackend::SetFillMode(eFillMode state)
	{
		if (glcontext.rasterizer.fillMode != state)
		{
			glcontext.rasterizer.fillMode = state;
			if (state == FM_SOLID)
			{
				GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
			}
			else
			{
				GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
			}
		}
	}

	void RenderBackend::SetViewport(int x, int y, int w, int h)
	{
		const glm::uvec4 v(x, y, w, h);
		if (glcontext.viewport == v) return;

		glcontext.viewport = v;
		GL_CHECK(glViewport(x, y, w, h));
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

		renderSystem.programManager->BindUniformBlock(UBB_VS_VIEW_PARAMS, view->VS_ViewParams);
		renderSystem.programManager->BindUniformBlock(UBB_FS_VIEW_PARAMS, view->FS_ViewParams);
#if 0
		float bw = (float)x / renderGlobals.bloomDownsampleLevel;
		float bh = (float)y / renderGlobals.bloomDownsampleLevel;

		renderSystem.programManager->SetSharedUniform(0, vec4{ bw, bh, 1.0f / bw, 1.0f / bh });
		renderSystem.programManager->UpdateCommonUniform();
#endif
		RenderDeferred_GBuffer();
		if (engineConfig.r_ssao)
		{
			RenderSSAO();
		}
		RenderDeferred_Lighting();
		RenderEmissive();
		RenderBloom();
		//RenderBloom_PBR();
		RenderHDRtoLDR();

		Framebuffer::Unbind();

		if (engineConfig.r_fxaa)
		{
			RenderAA();
		}
		else
		{
			uboSharedData_t parms{};
			parms.params[0].x = static_cast<float>(engineConfig.r_pp_offset);
			parms.params[0].y = static_cast<float>(engineConfig.r_pp);
			AllocSharedUbo(parms.params);
			SetCurrentTextureUnit(0);
			globalImages.defaultImage->Bind();
			renderSystem.programManager->UseProgram(PRG_KERNEL);
			R_DrawSurf(&unitTriSurface);
		}


#if 0
		GLsizei HalfWidth = (GLsizei)(x / 2);
		GLsizei HalfHeight = (GLsizei)(y / 2);
		if (engineConfig.r_ssao)
		{

			Framebuffer* target = globalFramebuffers.ssaoblurFBO[1];

			target->BindForReading();
			target->SetReadBuffer(0);
			target->BlitColorBuffer(
				0, 0, HalfWidth, HalfHeight,
				0, 0, HalfWidth/2, HalfHeight/2
			);
		}

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
		if (mode == CULL_NONE && glcontext.rasterizer.cullEnabled)
		{
			glcontext.rasterizer.cullEnabled = false;
			glDisable(GL_CULL_FACE);
			return;
		}
		if (mode == CULL_NONE && !glcontext.rasterizer.cullEnabled) return;

		if (!glcontext.rasterizer.cullEnabled)
		{
			glEnable(GL_CULL_FACE);
			glcontext.rasterizer.cullEnabled = true;
		}

		if (mode != glcontext.rasterizer.currentCullMode)
		{
			glcontext.rasterizer.currentCullMode = mode;
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

	void RenderBackend::SetBlendingState(const blendingState_t& state)
	{
		if (glcontext.blendState == state)
		{
			return;
		}
		if (glcontext.blendState.enabled != state.enabled)
		{
			glcontext.blendState.enabled = state.enabled;
			if (state.enabled)
			{
				glEnable(GL_BLEND);
			}
			else
			{
				glDisable(GL_BLEND);
			}
		}
		if (memcmp(&glcontext.blendState.opts, &state.opts, sizeof(state.opts)) != 0)
		{
			memcpy(&glcontext.blendState.opts, &state.opts, sizeof(state.opts));
			GLenum srcCol;
			GLenum dstCol;
			GLenum srcAlpha;
			GLenum dstAlpha;

			srcCol = GL_map_blendFunc(state.opts.colSrc);
			dstCol = GL_map_blendFunc(state.opts.colDst);
			srcAlpha = GL_map_blendFunc(state.opts.alphaSrc);
			dstAlpha = GL_map_blendFunc(state.opts.alphaDst);

			GL_CHECK(glBlendFuncSeparate(srcCol, dstCol, srcAlpha, dstAlpha));
		}
		if (glcontext.blendState.opts.colorOp != state.opts.colorOp || glcontext.blendState.opts.alphaOp != state.opts.alphaOp)
		{
			glcontext.blendState.opts.colorOp = state.opts.colorOp;
			glcontext.blendState.opts.alphaOp = state.opts.alphaOp;
			GLenum alphaOp;
			GLenum colOp;

			alphaOp = GL_map_blendEq(state.opts.alphaOp);
			colOp = GL_map_blendEq(state.opts.colorOp);
			GL_CHECK(glBlendEquationSeparate(alphaOp, colOp));
		}
	}

	void R_DrawSurf(const drawSurf_t* surf)
	{
		// setup vertex/index buffers
		renderSystem.vertexCache->BindVertexBuffer(surf->vertexCache, 0, sizeof(drawVert_t));
		renderSystem.vertexCache->BindIndexBuffer(surf->indexCache);

		IndexBuffer idx;
		const GLenum mode = surf->frontEndGeo ? GL_map_topology(surf->frontEndGeo->topology) : GL_TRIANGLES;

		if (mode == GL_TRIANGLES && renderSystem.vertexCache->GetIndexBuffer(surf->indexCache, idx))
		{
			GL_CHECK(glDrawElements(
				mode,
				surf->numIndex,
				GL_UNSIGNED_SHORT,
				(void*)idx.GetOffset()));
		}
		else
		{
			GL_CHECK(glDrawArrays(mode, 0, surf->frontEndGeo->numVerts));
		}

		perfCounters.drawElements++;
		perfCounters.drawIndexes += surf->numIndex;

	}
	void RenderBackend::RenderDepthPass()
	{
		using namespace glm;

		if (view == nullptr) return;
		depthState_t ds = glcontext.depthState;

		SetViewport(view->viewport.x, view->viewport.y, view->viewport.w, view->viewport.h);
		ds.depthMask = true;
		SetDepthState(ds);
		SetWriteMask(glm::bvec4{ false });

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

			renderSystem.programManager->BindUniformBlock(UBB_VS_DRAW_PARAMS, surf->space->VS_DrawParams);

			R_DrawSurf(surf);
		}
	}

	void RenderBackend::RenderShadow(const viewLight_t* light)
	{
		using namespace glm;

		globalFramebuffers.shadowFBO->Bind();
		depthState_t ds = glcontext.depthState;
		ds.depthMask = true;
		ds.func = CMP_LEQ;
		SetDepthState(ds);
		auto ss = glcontext.stencilState;
		ss.enabled = false;
		SetStencilState(ss);
		
		SetViewport(0, 0, renderGlobals.shadowResolution, renderGlobals.shadowResolution);

		Clear(false, true, false);
		SetWriteMask(glm::bvec4{ false });

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
				|| stage.images[IMU_EMISSIVE] != renderSystem.imageManager->globalImages.whiteImage) continue;
				**/

			renderSystem.programManager->BindUniformBlock(UBB_VS_DRAW_PARAMS, surf->space->VS_DrawParams);
			R_DrawSurf(surf);
		}

		//Framebuffer::Unbind();
	}

	void RenderBackend::RenderDeferred_GBuffer()
	{
		using namespace glm;

		globalFramebuffers.GBufferFBO->Bind();
		renderSystem.programManager->UseProgram(PRG_DEFERRED_GBUFFER_MR);
	
		SetDepthState({ true, true, CMP_LEQ });
		SetWriteMask(bvec4{ true });

		blendingState_t blendState = glcontext.blendState;
		blendState.enabled = false;

		SetBlendingState(blendState);
		stencilState_t ss = glcontext.stencilState;
		ss.enabled = false;
		ss.mask = 0;
		SetStencilState(ss);

		SetViewport(view->viewport.x, view->viewport.y, view->viewport.w, view->viewport.h);

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
				if (surf->space->shadowOnly) continue;

				if (glm::any(glm::greaterThan(vec3(stage.emissiveScale), vec3(0.0f))) || stage.images[IMU_EMISSIVE] != globalImages.whiteImage) continue;
				
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

				renderSystem.programManager->BindUniformBlock(UBB_VS_DRAW_PARAMS, surf->space->VS_DrawParams);
				renderSystem.programManager->BindUniformBlock(UBB_FS_DRAW_PARAMS, surf->FS_DrawParams[ACTIVE_STAGE]);

				R_DrawSurf(surf);
			}
		}
	}

	/*
	metro LL method
	1.
	==============
	Detph: test: enabled, write: disabled, func: LESEQUAL
	Stencil: enabled, wmask: FF, rmask: FF, Ref: 01, func: always, fail: keep, zfail: replace, pass: dec
	Blend: disabled
	Colormask: false
	Cull: FRONT

	2.
	==============
	Detph: test: enabled, write: disabled, func: LESEQUAL
	Stencil: enabled, wmask: FF, rmask: FF, Ref: 00, func: always, fail: keep, zfail: replace, pass: keep
	Blend: disabled
	Colormask: false
	Cull: BACK

	3.
	==============
	Detph: test: enabled, write: disabled, func: ALWAYS
	Stencil: enabled, wmask: 00, rmask: FF, Ref: 01, func: equal, fail: keep, zfail: keep, pass: keep
	Blend: enabled
	Colormask: true
	Cull: FRONT

	*/
	void RenderBackend::RenderDeferred_Lighting()
	{
		using namespace glm;

		SetViewport(view->viewport.x, view->viewport.y, view->viewport.w, view->viewport.h);

		globalFramebuffers.hdrFBO->Bind();
		
		/*
		globalFramebuffers.GBufferFBO->BindForReading();
		globalFramebuffers.GBufferFBO->BlitDepthBuffer(0, 0, view->viewport.w, view->viewport.h, 0, 0, view->viewport.w, view->viewport.h);
		globalFramebuffers.hdrFBO->BindForReading();
		*/

		const bvec4 bfalse{ false };
		const bvec4 btrue{ true };

		SetWriteMask(btrue);
		Clear(true, false, true);

		SetCurrentTextureUnit(IMU_DIFFUSE);	globalImages.GBufferAlbedo->Bind();
		SetCurrentTextureUnit(IMU_FRAGPOS);	globalImages.GBufferFragPos->Bind();
		SetCurrentTextureUnit(IMU_NORMAL);	globalImages.GBufferNormal->Bind();
		SetCurrentTextureUnit(IMU_AORM);	globalImages.GBufferSpec->Bind();
		SetCurrentTextureUnit(IMU_SHADOW);	globalImages.Shadow->Bind();
		SetCurrentTextureUnit(IMU_DEFAULT);	globalImages.ssaoblur[1]->Bind();

		blendingState_t blendState{};
		blendState.enabled = false;
		blendState.opts.alphaOp = BOP_ADD;
		blendState.opts.colorOp = BOP_ADD;
		blendState.opts.alphaDst = BFUNC_ONE;
		blendState.opts.colDst = BFUNC_ONE;
		blendState.opts.alphaSrc = BFUNC_ONE;
		blendState.opts.colSrc = BFUNC_ONE;
		SetBlendingState(blendState);
		
		stencilState_t stencilSt{};
		stencilSt.enabled = false;
		stencilSt.separate = false;


		// render sun & AO
		vec4 params[JSR_R_SHARED_NUMPARAMS]{};

		auto* pm = renderSystem.programManager;
		pm->UseProgram(PRG_DEFERRED_DIR_LIGHT);
		params[0].x = engineConfig.r_ssao ? 1.0f : 0.0f;
		AllocSharedUbo(params);

		SetCullMode(CULL_NONE);
		SetStencilState(stencilSt);
		SetDepthState({ true, false, CMP_ALWAYS });
		renderSystem.programManager->BindUniformBlock(UBB_LIGHT_DATA, view->viewSunLight->lightData);
		R_DrawSurf(&unitTriSurface);
		
		renderSystem.programManager->UseProgram(PRG_DEFERRED_LIGHT);

		for (const auto* light = view->viewLights; light != nullptr; light = light->next)
		{
			pm->BindUniformBlock(UBB_LIGHT_DATA, light->lightData);
#if 1
			if (light->type == LIGHT_SPOT)
			{
				RenderShadow(light);
				globalFramebuffers.hdrFBO->Bind();
				SetViewport(view->viewport.x, view->viewport.y, view->viewport.w, view->viewport.h);
				pm->UseProgram(PRG_DEFERRED_LIGHT);
			}
#endif
			pm->BindUniformBlock(UBB_VS_DRAW_PARAMS, light->VS_DrawParams);

			SetCullMode(CULL_FRONT);
			stencilSt.mask = 0xff;
			stencilSt.ref = 0x01;
			stencilSt.stencilFunc = CMP_ALWAYS;
			stencilSt.fail = SO_KEEP;
			stencilSt.zfail = SO_REPLACE;
			stencilSt.pass = SO_DEC;
			stencilSt.enabled = true;
			SetStencilState(stencilSt);
			SetWriteMask(bfalse);
			SetDepthState({ true, false, CMP_LEQ });

			if (light->type == LIGHT_POINT)
			{
				R_DrawSurf(&unitSphereSurface);
			}
			else if (light->type == LIGHT_SPOT)
			{
				R_DrawSurf(&unitConeSurface);
			}

			SetCullMode(CULL_BACK);
			stencilSt.mask = 0xff;
			stencilSt.ref = 0x00;
			stencilSt.stencilFunc = CMP_ALWAYS;
			stencilSt.fail = SO_KEEP;
			stencilSt.zfail = SO_REPLACE;
			stencilSt.pass = SO_KEEP;
			stencilSt.enabled = true;
			SetStencilState(stencilSt);
			SetWriteMask(bfalse);
			SetDepthState({ true, false, CMP_LEQ });

			if (light->type == LIGHT_POINT)
			{
				R_DrawSurf(&unitSphereSurface);
			}
			else if (light->type == LIGHT_SPOT)
			{
				R_DrawSurf(&unitConeSurface);
			}

			SetCullMode(CULL_FRONT);
			stencilSt.mask = 0x00;
			stencilSt.ref = 0x01;
			stencilSt.stencilFunc = CMP_EQ;
			stencilSt.fail = SO_KEEP;
			stencilSt.zfail = SO_KEEP;
			stencilSt.pass = SO_KEEP;
			stencilSt.enabled = true;
			SetStencilState(stencilSt);
			SetWriteMask(btrue);
			SetDepthState({ true, false, CMP_ALWAYS });
			blendState.enabled = true;
			SetBlendingState(blendState);

			if (light->type == LIGHT_POINT)
			{
				R_DrawSurf(&unitSphereSurface);
			}
			else if (light->type == LIGHT_SPOT)
			{
				R_DrawSurf(&unitConeSurface);
			}
		}
		stencilSt.enabled = false;
		SetStencilState(stencilSt);
		//glDisable(GL_STENCIL_TEST);
	}

	void RenderBackend::RenderEmissive()
	{
		using namespace glm;

		globalFramebuffers.hdrFBO->Bind();
		renderSystem.programManager->UseProgram(PRG_EMISSIVE);

		depthState_t ds{};
		ds.enabled = true;
		ds.depthMask = false;
		ds.func = CMP_LEQ;
		SetDepthState(ds);
		SetWriteMask(bvec4{ true });

		blendingState_t blendState = glcontext.blendState;
		blendState.enabled = false;
		SetBlendingState(blendState);

		SetViewport(view->viewport.x, view->viewport.y, view->viewport.w, view->viewport.h);

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
				if (surf->space->shadowOnly) continue;

				if (glm::all(glm::equal(vec3(stage.emissiveScale), vec3(0.0f))) && stage.images[IMU_EMISSIVE] == globalImages.whiteImage) continue;

				// setup textures
				renderSystem.backend->SetCurrentTextureUnit(IMU_EMISSIVE);
				stage.images[IMU_EMISSIVE]->Bind();
				SetCullMode(stage.cullMode);

				renderSystem.programManager->BindUniformBlock(UBB_VS_DRAW_PARAMS, surf->space->VS_DrawParams);
				renderSystem.programManager->BindUniformBlock(UBB_FS_DRAW_PARAMS, surf->FS_DrawParams[ACTIVE_STAGE]);

				R_DrawSurf(surf);
			}
		}


#if 0
		using namespace glm;

		SetViewport(view->viewport.x, view->viewport.y, view->viewport.w, view->viewport.h);
		renderSystem.programManager->UseProgram(PRG_GAUSS_FILTER);

		glm::vec2 vDirection{ 1.0f,0.0f };

		SetCurrentTextureUnit(0);
		for (int i = 0; i < 3; ++i)
		{
			vDirection.x = 1.0f;
			vDirection.y = 0.0f;
			globalFramebuffers.bloomFBO[1]->Bind();
			globalImages.HDRblur[0]->Bind();
			renderSystem.programManager->SetUniform(PRG_GAUSS_FILTER, "g_vDirection", vDirection);
			R_DrawSurf(&unitRectSurface);
			vDirection.x = 0.0f;
			vDirection.y = 1.0f;
			globalFramebuffers.bloomFBO[0]->Bind();
			globalImages.HDRblur[1]->Bind();
			renderSystem.programManager->SetUniform(PRG_GAUSS_FILTER, "g_vDirection", vDirection);
			R_DrawSurf(&unitRectSurface);
		}

		SetViewport(view->viewport.x, view->viewport.y, view->viewport.w, view->viewport.h);
		renderSystem.programManager->UseProgram(PRG_TEXTURED);
		globalFramebuffers.hdrFBO->Bind();
		globalImages.HDRblur[0]->Bind();
		blendState.enabled = true;
		blendState.opts.alphaDst = BFUNC_ONE;
		blendState.opts.colDst = BFUNC_ONE;
		blendState.opts.alphaSrc = BFUNC_ONE;
		blendState.opts.colSrc = BFUNC_ONE;
		blendState.opts.alphaOp = BOP_ADD;
		blendState.opts.colorOp = BOP_ADD;
		SetBlendingState(blendState);
		ds.enabled = true;
		ds.func = CMP_ALWAYS;
		ds.depthMask = false;
		SetDepthState(ds);
		R_DrawSurf(&unitRectSurface);
#endif
	}

	void RenderBackend::RenderBloom()
	{
		using namespace glm;

		if (!engineConfig.r_bloom) return;

		auto scr = renderSystem.GetScreenSize();
		stencilState_t ss = glcontext.stencilState;
		depthState_t ds = glcontext.depthState;
		ds.enabled = true;
		ds.depthMask = false;
		ds.func = CMP_ALWAYS;
		SetDepthState(ds);
		ss.enabled = false;
		SetStencilState(ss);

		SetCullMode(CULL_NONE);

		blendingState_t bs = glcontext.blendState;
		bs.enabled = false;
		renderSystem.programManager->UseProgram(PRG_BLOOM_FILTER);
		SetCurrentTextureUnit(0);

		float w = scr.x / 2.0f;
		float h = scr.y / 2.0f;
		SetViewport(0, 0, (int)w,(int)h);
		

		globalFramebuffers.bloomFBO[0]->Bind();
		globalImages.HDRaccum->Bind();
		R_DrawSurf(&unitTriSurface);

		w = w / 2.0f;
		h = h / 2.0f;
		SetViewport(0, 0, (int)w, (int)h);

		globalFramebuffers.bloomFBO[1]->Bind();
		globalImages.HDRbloom[0]->Bind();
		R_DrawSurf(&unitTriSurface);

		auto* pm = renderSystem.programManager;
		pm->UseProgram(PRG_GAUSS_FILTER);

		uboSharedData_t p{};

		for (int i = 0; i < 2; ++i)
		{
			p.params[0].x = 1.0f;
			p.params[0].y = 0.0f;
			globalFramebuffers.blurFBO[0]->Bind();
			if (i == 0)
			{
				globalImages.HDRbloom[0]->Bind();
			}
			else
			{
				globalImages.HDRblur[1]->Bind();
			}

			//pm->UpdateSharedUniform();
			AllocSharedUbo(p.params);

			R_DrawSurf(&unitTriSurface);

			globalFramebuffers.blurFBO[1]->Bind();
			globalImages.HDRblur[0]->Bind();

			p.params[0].x = 0.0f;
			p.params[0].y = 1.0f;
			AllocSharedUbo(p.params);

			R_DrawSurf(&unitTriSurface);
		}
	}

	void RenderBackend::RenderBloom_PBR()
	{
		// downsample

		SetCurrentTextureUnit(0);
		renderSystem.programManager->UseProgram(PRG_DOWNSAMPLE);
		stencilState_t stencil = glcontext.stencilState;
		blendingState_t blend = glcontext.blendState;
		depthState_t Z = glcontext.depthState;

		blend.enabled = false;
		SetBlendingState(blend);
		SetCullMode(CULL_NONE);
		stencil.enabled = false;
		SetStencilState(stencil);
		Z.depthMask = false;
		Z.func = CMP_ALWAYS;
		SetDepthState(Z);

		Image* im = globalImages.HDRaccum;
		uboSharedData_t p{};
		for (int i = 0; i < globalFramebuffers.PBRbloomFBO.size(); ++i)
		{
			const int x = im->opts.sizeX;
			const int y = im->opts.sizeY;
			const float fx = float(x);
			const float fy = float(y);
			p.params[0] = { fx, fy, 1.0f / fx, 1.0f / fy };
			p.params[1].x = float(i);
			AllocSharedUbo(p.params);
			globalFramebuffers.PBRbloomFBO[i]->Bind();
			im->Bind();
			SetViewport(0, 0, globalImages.PBRbloom[i]->opts.sizeX, globalImages.PBRbloom[i]->opts.sizeY);
			R_DrawSurf(&unitTriSurface);
			
			im = globalImages.PBRbloom[i];
		}

		renderSystem.programManager->UseProgram(PRG_UPSAMPLE);
		blend.enabled = true;
		blend.opts.colorOp = BOP_ADD;
		blend.opts.colSrc = BFUNC_ONE;
		blend.opts.colDst = BFUNC_ONE;
		SetBlendingState(blend);

		for (int i = globalFramebuffers.PBRbloomFBO.size() - 1; i > 0; --i)
		{
			globalImages.PBRbloom[i]->Bind();
			globalFramebuffers.PBRbloomFBO[i - 1]->Bind();
			p.params[0].x = renderGlobals.bloomUpsampleRadius / float(std::max(globalImages.PBRbloom[i - 1]->opts.sizeX, globalImages.PBRbloom[i - 1]->opts.sizeY));
			AllocSharedUbo(p.params);
			SetViewport(0, 0, globalImages.PBRbloom[i - 1]->opts.sizeX, globalImages.PBRbloom[i - 1]->opts.sizeY);

			R_DrawSurf(&unitTriSurface);
		}
		blend.enabled = false;
		SetBlendingState(blend);

	}

	void RenderBackend::RenderSSAO()
	{
		using namespace glm;

		int w, h;
		GetScreenSize(w, h);
		const int hw = int(w * renderGlobals.ssaoResolutionScale);
		const int hh = int(h * renderGlobals.ssaoResolutionScale);

		SetViewport(0, 0, hw, hh);

		auto ds = glcontext.depthState;
		ds.depthMask = false;
		ds.func = CMP_ALWAYS;
		SetDepthState(ds);
		SetWriteMask(bvec4(true));
		SetCullMode(CULL_NONE);
		blendingState_t blendState = glcontext.blendState;
		blendState.enabled = false;
		SetBlendingState(blendState);

		SetCurrentTextureUnit(IMU_FRAGPOS);
		globalImages.GBufferFragPos->Bind();
		SetCurrentTextureUnit(IMU_NORMAL);
		globalImages.GBufferNormal->Bind();
		SetCurrentTextureUnit(IMU_DEFAULT);
		globalImages.ssaoNoise->Bind();

		vec4 params[JSR_R_SHARED_NUMPARAMS]{};
		auto* pm = renderSystem.programManager;
		globalFramebuffers.ssaoFBO->Bind();
		pm->UseProgram(PRG_SSAO_GEN);
		params[0] = { float(hw),float(hh),1.0f / hw,1.0f / hh };
		params[1].x = engineConfig.r_ssao_radius;
		params[1].y = engineConfig.r_ssao_bias;
		params[1].z = engineConfig.r_ssao_str;
		params[1].w = 2.0f / renderGlobals.ssaoResolutionScale;
		AllocSharedUbo(params);
		R_DrawSurf(&unitTriSurface);
#if 0
		pm->UseProgram(PRG_KERNEL);
		pm->g_backendData.params[0].x = 1.0f / float(hw);
		pm->g_backendData.params[0].y = 1.0f / float(hh);
		pm->g_backendData.params[0].z = 0.0f;
		pm->UpdateSharedUniform();
		SetCurrentTextureUnit(0);

		globalFramebuffers.ssaoblurFBO[0]->Bind();
		globalImages.ssaoMap->Bind();
		R_DrawSurf(&unitRectSurface);
		globalFramebuffers.ssaoblurFBO[1]->Bind();
		globalImages.ssaoblur[0]->Bind();
		R_DrawSurf(&unitRectSurface);

#else
		pm->UseProgram(PRG_GAUSS_FILTER);
		params[0].x = 0.0f;
		params[0].y = 1.0f;
		AllocSharedUbo(params);
		SetCurrentTextureUnit(0);
		globalFramebuffers.ssaoblurFBO[0]->Bind();
		globalImages.ssaoMap->Bind();
		R_DrawSurf(&unitTriSurface);

		globalFramebuffers.ssaoblurFBO[1]->Bind();
		globalImages.ssaoblur[0]->Bind();
		params[0].x = 1.0f;
		params[0].y = 0.0f;
		AllocSharedUbo(params);
		R_DrawSurf(&unitTriSurface);
#endif
	}

	void RenderBackend::RenderHDRtoLDR()
	{
		using namespace glm;

		int w, h;
		GetScreenSize(w, h);
		SetViewport(0, 0, w, h);
		auto ds = glcontext.depthState;
		ds.depthMask = false;
		ds.func = CMP_ALWAYS;
		SetDepthState(ds);
		SetWriteMask(bvec4(true));
		SetCullMode(CULL_NONE);

		blendingState_t blendState = glcontext.blendState;
		blendState.enabled = false;
		SetBlendingState(blendState);

		SetCurrentTextureUnit(IMU_HDR);
		globalImages.HDRaccum->Bind();
		SetCurrentTextureUnit(IMU_DIFFUSE);
		globalImages.GBufferAlbedo->Bind();
		SetCurrentTextureUnit(IMU_EMISSIVE);
		globalImages.HDRblur[1]->Bind();
		//globalImages.PBRbloom[0]->Bind();
		SetCurrentTextureUnit(IMU_AORM);
		if (engineConfig.r_ssao)
		{
			globalImages.ssaoblur[1]->Bind();
		}
		else
		{
			globalImages.whiteImage->Bind();
		}

		renderSystem.programManager->UseProgram(PRG_PP_HDR);
		globalFramebuffers.defaultFBO->Bind();

		R_DrawSurf(&unitTriSurface);
	}

	void RenderBackend::RenderAA()
	{
		using namespace glm;

		int w, h;
		GetScreenSize(w, h);
		SetViewport(0, 0, w, h);
		auto ds = glcontext.depthState;
		ds.depthMask = false;
		ds.func = CMP_ALWAYS;
		SetDepthState(ds);
		SetWriteMask(bvec4(true));
		SetCullMode(CULL_NONE);
		blendingState_t blendState = glcontext.blendState;
		blendState.enabled = false;
		SetBlendingState(blendState);
		Framebuffer::Unbind();

		SetCurrentTextureUnit(0);
		globalImages.defaultImage->Bind();
		renderSystem.programManager->UseProgram(PRG_FXAA3);
		R_DrawSurf(&unitTriSurface);
		//R_DrawFullscreenTri();
	}

	void RenderBackend::AllocSharedUbo(const glm::vec4* data)
	{
		const int size = JSR_R_SHARED_NUMPARAMS * sizeof(*data);
		const int numBytes = (size + (glconfig.uniformBufferOffsetAligment - 1)) & ~(glconfig.uniformBufferOffsetAligment - 1);

		if (glcontext.sharedUboAlloced + numBytes >= glcontext.sharedUboSize)
		{
			glcontext.sharedUboAlloced = 0;
		}
		int offset = glcontext.sharedUboAlloced;
		glcontext.sharedUboAlloced += numBytes;
		glcontext.sharedUbo.Update(data, offset, size);
		glcontext.sharedUbo.BindRange(UBB_SHARED_DATA, offset, size);
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

}