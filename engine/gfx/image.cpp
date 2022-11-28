#include <stdexcept>
#include <filesystem>
#include "engine/gfx/image.h"
#include "stb_image.h"
#include "stb_image_resize.h"

namespace gfx {

	namespace fs = std::filesystem;

	uint ImageSet::levels() const
	{
		return levels_;
	}
	bool ImageSet::empty() const
	{
		return levels_ == 0;
	}
	Image const& ImageSet::at(uint level) const
	{
		return images_.at(level);
	}
	Image const& ImageSet::operator[](uint level) const
	{
		return images_[level];
	}
	TextureShape ImageSet::shape() const
	{
		return shape_;
	}
	void ImageSet::setShape(TextureShape ashape)
	{
		shape_ = ashape;
	}
	ImageSet& ImageSet::fromFile(String const& afilename)
	{

		const fs::path path(afilename);
		const String extension = path.extension().generic_string();

		if (extension == ".ktx" || extension == ".ktx2")
		{
			return fromKtxFile(afilename);
		}

		//...
		return ImageSet();
	}
	ImageSet& ImageSet::fromKtxFile(String const& afilename)
	{
		return ImageSet();
	}
}