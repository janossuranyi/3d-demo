#pragma once
#include "./RenderBackend.h"
#include "./VertexCache.h"

namespace jsr {

	class RenderSystem
	{
	public:
		RenderBackend* backend;
		VertexCache* vertexCache;

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
