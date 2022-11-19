#pragma once

#include <atomic>
#include "common.h"
#include "gfx/renderer.h"
#include "gfx/shader_manager.h"
#include "gfx/vertex_cache.h"

namespace ctx {

	class Context
	{
	public:
		const uint STATIC_SIZE = 128 * 1024 * 1024;
		const uint FRAME_SIZE = 16 * 1024 * 1024;

		static Context* default();
		gfx::Renderer* hwr();
		gfx::ShaderManager* shaderManager();
		gfx::VertexCache* vertexCache();
		~Context() = default;
	private:
		Context();
		std::unique_ptr<gfx::Renderer> hwr_;
		std::unique_ptr<gfx::ShaderManager> sm_;
		std::unique_ptr<gfx::VertexCache> vc_;
		static std::unique_ptr<Context> instance_;
		static std::atomic_bool ready_;
	};
}