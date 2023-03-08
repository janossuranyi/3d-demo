#ifndef JSE_TEXTURE_MANAGER_H
#define JSE_TEXTURE_MANAGER_H

namespace js {

	using TextureByIdMap	= std::unordered_map<std::string, Texture>;

	class TextureManager: public NonMovable {
	private:
		TextureByIdMap	texture_map_;
		GfxRenderer*	hwr_;
	public:
		TextureManager() = delete;
		TextureManager(GfxRenderer* gfxRenderer);
		Result LoadTexture(JsString fileName, Texture** tex);
	};
}
#endif // !JSE_TEXTURE_MANAGER_H
