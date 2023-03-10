#include "JSE.h"
#include "JSE_GfxCoreGL46.h"

namespace js {
	
	Engine::Engine()
	{
		config_ = new Config();
		config_->fullscreen = false;
		config_->screen_width = 1280;
		config_->screen_height = 720;
		init();
	}

	Engine::Engine(Config* conf)
	{
		config_  = new Config();
		*config_ = *conf;
		init();
	}

	Engine::~Engine()
	{
		Info("Engine shutting down...");
		Shutdown();

		delete textureManager_;
		delete vertexCache_;
		delete render_backend_;
		delete gfxCore_;
		delete inputManager_;
		delete config_;
	}

	GfxRenderer* Engine::GetRenderer()
	{
		return render_backend_;
	}

	InputManager* Engine::GetInputManager()
	{
		return inputManager_;
	}

	const Config* Engine::GetConfig() const
	{
		return config_;
	}

	GfxCore* Engine::GetCore()
	{
		return gfxCore_;
	}

	VertexCache* Engine::GetVertexCache()
	{
		return vertexCache_;
	}

	TextureManager* Engine::GetTextureManager()
	{
		return textureManager_;
	}

	void Engine::init()
	{
		Info("Engine startup...");
		inputManager_		= new InputManager();
		gfxCore_			= new GfxCoreGL();
		render_backend_		= new GfxRenderer(gfxCore_, 64*1024*1024);
		render_backend_->InitCore(config_->screen_width, config_->screen_height, config_->fullscreen, true);

		vertexCache_		= new VertexCache(render_backend_);
		textureManager_		= new TextureManager(render_backend_);
		Info("Engine startup\tDone");
	}
}