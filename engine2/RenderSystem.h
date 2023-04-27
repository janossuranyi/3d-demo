#pragma once
#include "./RenderBackend.h"
#include "./VertexCache.h"
#include "./ImageManager.h"
namespace jsr {

	class RenderSystem
	{
	public:
		VertexCache* vertexCache;
		ProgramManager* programManager;
		ImageManager* imageManager;
		RenderBackend* backend;
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
