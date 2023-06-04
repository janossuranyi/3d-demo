#pragma once
#include "./RenderBackend.h"
#include "./VertexCache.h"
#include "./ImageManager.h"
#include "./Material.h"
#include "./Model.h"
#include "./Camera.h"

#define CACHE_LINE_SIZE 64

namespace jsr {

	struct renderGlobals_t
	{
		unsigned int shadowResolution;
		float shadowScale;
		float shadowBias;
		float defaultExpAttn;
		float defaultLinearAttn;
		glm::vec3 ambientColor;
		float ambientScale;

		renderGlobals_t() :
			shadowResolution(1024),
			shadowScale(0.8f),
			defaultExpAttn(1.0f),
			defaultLinearAttn(0.0f),
			ambientColor(1.0f),
			ambientScale(.01f),
			shadowBias(0.0001f) {}
	};

	struct frameData_t {
		
		std::atomic_int				used;
		uint8_t*					frameMemory;
		emptyCommand_t*				cmdTail;
		emptyCommand_t*				cmdHead;
	};

	uint8_t* R_FrameAlloc(uint32_t bytes);
	emptyCommand_t* R_GetCommandBuffer(uint32_t size);
	emptyCommand_t* R_SwapCommandBuffers(bool swap = true);
	void R_ResetCommandBuffer();
	void R_InitCommandBuffers();
	void R_ShutdownCommandBuffers();

	class RenderSystem
	{
	public:
		VertexCache* vertexCache;
		ProgramManager* programManager;
		ImageManager* imageManager;
		RenderBackend* backend;
		MaterialManager* materialManager;
		ModelManager* modelManager;

		RenderSystem();
		~RenderSystem();

		bool Init();
		void Shutdown();
		bool IsInitialized() const;
		void RenderFrame(const emptyCommand_t* cmds);
		emptyCommand_t const* SwapCommandBuffer_BeginNewFrame(bool smpMode);
		glm::vec2 GetScreenSize() const;
		viewDef_t* view;

		Material* defaultMaterial;

		drawSurf_t unitRectSurface_;
		drawSurf_t unitCubeSurface_;
		drawSurf_t unitSphereSurface_;
		surface_t* unitRectTris;
		surface_t* unitCubeTris;
		surface_t* unitSphereTris;

	private:
		bool initialized;
		int	 frameNum;
	};

	extern RenderSystem renderSystem;
	extern renderGlobals_t renderGlobals;
}
