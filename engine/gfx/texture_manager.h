#ifndef GFX_TEXTURE_MANAGER_H
#define GFX_TEXTURE_MANAGER_H

#include "engine/gfx/renderer.h"

namespace gfx {
	class TextureManager
	{
	public:
		TextureManager() = default;
		TextureManager(Renderer* R);
		TextureManager(TextureManager&) = delete;
		TextureManager(TextureManager&&) = default;
		TextureManager& operator=(TextureManager&) = delete;
		TextureManager& operator=(TextureManager&&) = default;

		void setRenderer(Renderer* R);

		TextureHandle operator[](const String& id) const;
		TextureHandle get(const String& id) const;
		TextureHandle createFromResource(const String& id, bool srgb = true);
		bool exists(const String& id) const;
		bool remove(const String& id);
	private:
		Renderer* R_;
		HashMap<String, TextureHandle> texture_map_;
	};

}
#endif // !GFX_TEXTURE_MANAGER_H
