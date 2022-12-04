#include <stdexcept>
#include <filesystem>

#include "engine/gfx/image.h"
#include "engine/gfx/memory.h"
#include "stb_image.h"
#include "stb_image_resize.h"

namespace gfx {

	namespace fs = std::filesystem;

	bool ImageSet::flipImage() const
	{
		return flip_image_;
	}

	void ImageSet::setFlipImage(bool aflip)
	{
		flip_image_ = aflip;
	}

	uint ImageSet::levels() const
	{
		return images_.size();
	}
	bool ImageSet::empty() const
	{
		return images_.empty();
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
	TextureFormat ImageSet::format() const
	{
		return format_;
	}
	bool ImageSet::srgb() const
	{
		return srgb_;
	}
	void ImageSet::setShape(TextureShape ashape)
	{
		shape_ = ashape;
	}
	void ImageSet::setFormat(TextureFormat aformat)
	{
		format_ = aformat;
	}
	ImageSet& ImageSet::fromFile(String const& afilename, bool asrgb)
	{		
		stbi_set_flip_vertically_on_load(flip_image_);

		format_ = TextureFormat::RGBA8;
		srgb_ = asrgb;

		int x, y, n;
		stbi_uc* image = stbi_load(afilename.c_str(), &x, &y, &n, STBI_rgb_alpha);
		if (image)
		{
			alpha_ = (n == 4);
			images_.emplace_back(Image{
				static_cast<ushort>(x),
				static_cast<ushort>(y),
				Memory(image, x * y * 4) });

			shape_ = (y == 1 ? TextureShape::D1 : TextureShape::D2);

			stbi_image_free(image);
		}

		return *this;
	}
	Vector<ImageSet> ImageSet::fromFiles(Vector<String> const& afilenames, bool asrgb, bool automipmaps)
	{
		Vector<ImageSet> ret;

		for (auto& e : afilenames)
		{
			ImageSet set{};
			set.fromFile(e, asrgb);
			if (!set.empty() && automipmaps)
			{
				set.generateMipmaps();
			}
			ret.emplace_back(std::move(set));
		}

		return ret;
	}
	ImageSet& ImageSet::set(ushort alevel, ushort awidth, ushort aheight, TextureFormat aformat, bool asrgb, Memory& adata)
	{
		if (levels() <= alevel)
		{
			images_.resize(alevel + 1);
		}
	
		format_ = aformat;
		srgb_ = asrgb;
		images_[alevel] = Image{ awidth,aheight,std::move(adata) };

		return *this;

	}
	ImageSet ImageSet::create(ushort awidth, ushort aheight, TextureFormat aformat, bool asrgb, Memory& adata)
	{
		ImageSet ret{};
		ret.set(0, awidth, aheight, aformat, asrgb, adata);

		return (ret);
	}
	ImageSet ImageSet::create(ushort awidth, ushort aheight, TextureFormat aformat)
	{
		return ImageSet::create(awidth, aheight, aformat, false, Memory());
	}
	void ImageSet::generateMipmaps()
	{
		if (empty()) return;

		// make sure to keep only level 0 texture
		if (levels() > 1)
		{
			images_.resize(1);
		}

		uint w = images_[0].width;
		uint h = images_[0].height;
		std::unique_ptr<uint8[]> input_buffer(new uint8[w * h * 4]);
		std::memcpy(input_buffer.get(), images_[0].data.data(), w * h * 4);
		const uint levels = std::floorf(std::log2f(static_cast<float>(std::max(images_[0].width, images_[0].height)))) + 1;

		for (uint k = 1; k < levels; ++k)
		{
			w = std::max(uint(w / 2), 1u);
			h = std::max(uint(h / 2), 1u);
			std::unique_ptr<uint8[]> output_buffer(new uint8[w * h * 4]);
			bool res = false;
			if (srgb_)
			{
				res = stbir_resize_uint8_srgb(input_buffer.get(), w * 2, h * 2, 0, output_buffer.get(), w, h, 0, 4, (alpha_ ? 3 : -1), 0);
			}
			else
			{
				res = stbir_resize_uint8(input_buffer.get(), w * 2, h * 2, 0, output_buffer.get(), w, h, 0, 4);
			}

			if (res)
			{
				images_.emplace_back(Image{
					static_cast<ushort>(w),
					static_cast<ushort>(h),
					Memory(output_buffer.get(), w * h * 4)});
			}

			input_buffer = std::move(output_buffer);
		}

	}
}