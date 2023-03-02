#ifndef JSE_ENGINE_H
#define JSE_ENGINE_H

namespace js {

	class Engine
	{
	public:
		Engine();
		Engine(Config* conf);
		~Engine();

		virtual bool Init() { return false; };
		virtual void Run() {};
		virtual void Done() {};

		GfxRenderer* GetRenderer();
		InputManager* GetInputManager();
		const Config* GetConfig() const;
		GfxCore* GetCore();
		VertexCache* GetVertexCache();
	private:
		void init_();
		GfxCore* gfxCore_;
		GfxRenderer* render_backend_;
		InputManager* inputManager_;
		VertexCache* vertexCache_;
		Config* config_;
	};

}
#endif // !JSE_ENGINE_H
