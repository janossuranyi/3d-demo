#pragma once
#include "./RenderBackend.h"
#include "./VertexCache.h"
#include "./ImageManager.h"
#include "./Material.h"
#include "./Model.h"

namespace jsr {

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
