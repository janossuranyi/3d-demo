#ifndef JSE_TEXTURE_MANAGER_H
#define JSE_TEXTURE_MANAGER_H

namespace js {

	using TextureByIdMap	= std::unordered_map<std::string, Texture>;

	class TextureManager: public NonMovable {
	private:
		TextureByIdMap	texture_map_;
		GfxRenderer*	hwr_;
		Result load_ktx(JsString filename, JseImageID image, const JseSamplerDescription& sampler, Texture* resTex);
	public:
		TextureManager() = delete;
		TextureManager(GfxRenderer* gfxRenderer);
		Result LoadTexture(JsString resourceName, const JseSamplerDescription& sampler, Texture* resTex);
	};
}
#endif // !JSE_TEXTURE_MANAGER_H
