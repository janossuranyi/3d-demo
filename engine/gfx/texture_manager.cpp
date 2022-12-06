#include <utility>
#include <stdexcept>
#include "texture_manager.h"
#include "resource/resource_manager.h"

namespace gfx {

	TextureManager::TextureManager(Renderer* R)
	{
		R_ = R;
	}

	void TextureManager::setRenderer(Renderer* R)
	{
		R_ = R;
	}

	TextureHandle TextureManager::operator[](const String& id) const
	{
		return get(id);
	}

	TextureHandle TextureManager::get(const String& id) const
	{
		auto res = texture_map_.find(id);
		if (res != std::end(texture_map_))
		{
			return res->second;
		}

		return TextureHandle();
	}

	TextureHandle TextureManager::createFromResource(const String& id, bool srgb, bool compressed)
	{
		if (exists(id)) return texture_map_[id];

		auto res = R_->createTexture(gfx::TextureWrap::Repeat, gfx::TextureFilter::LinearLinear, gfx::TextureFilter::Linear,
			rc::ResourceManager::get_resource(id), srgb, true, compressed, 1);

		texture_map_.emplace(id, res);

		return res;
	}

	bool TextureManager::exists(const String& id) const
	{
		return texture_map_.count(id) > 0;
	}

	bool TextureManager::remove(const String& id)
	{
		if (!R_) throw std::runtime_error("gfx::Renderer not set");

		auto res = texture_map_.find(id);
		if (res != std::end(texture_map_))
		{
			R_->deleteTexture(res->second);
			texture_map_.erase(id);

			return true;
		}

		return false;
	}

}
