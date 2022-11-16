#pragma once

#include "common.h"
#include "gfx/renderer.h"
#include "gfx/shader_manager.h"

namespace ctx {

	class Context
	{
	public:
		static Context* default();
		gfx::Renderer* hwr();
		gfx::ShaderManager* shaderManager();
		~Context() = default;
	private:
		Context();
		std::unique_ptr<gfx::Renderer> hwr_;
		std::unique_ptr<gfx::ShaderManager> sm_;
		static std::unique_ptr<Context> instance_;
	};
}