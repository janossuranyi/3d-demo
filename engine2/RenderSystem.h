#pragma once
#include "./RenderBackend.h"
#include "./VertexCache.h"
#include "./ImageManager.h"
#include "./Material.h"
#include "./Model.h"

#define CACHE_LINE_SIZE 64

namespace jsr {


	enum eRenderCommand
	{
		RC_NOP
	};

	struct emptyCommand_t
	{
		eRenderCommand command;
		emptyCommand_t* next;
	};

	struct frameData_t {
		
		std::atomic_int				frameMemoryPtr;
		uint8_t*					frameMemory;
		emptyCommand_t*				cmdTail;
		emptyCommand_t*				cmdHead;
	};

	uint8_t* R_FrameAlloc(uint32_t bytes);
	emptyCommand_t* R_GetCommandBuffer(uint32_t size);
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
		void Frame();

	private:
		bool initialized;
		int	 frameNum;
	};


	extern RenderSystem renderSystem;


}
