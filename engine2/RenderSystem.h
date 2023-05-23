#pragma once
#include "./RenderBackend.h"
#include "./VertexCache.h"
#include "./ImageManager.h"
#include "./Material.h"
#include "./Model.h"
#include "./Camera.h"

#define CACHE_LINE_SIZE 64

namespace jsr {

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
		void Frame(const emptyCommand_t* cmds);
		viewDef_t* view;

		Material* defaultMaterial;

		const int shadowResolution = 1024;
	private:
		bool initialized;
		int	 frameNum;
		RenderModel* unitrect;
	};

	extern RenderSystem renderSystem;

}
