#ifndef GFX_IMAGE_H
#define GFX_IMAGE_H

#include "engine/gfx/render_types.h"
#include "engine/gfx/memory.h"

namespace gfx {
	struct Image
	{
		ushort width;
		ushort height;
		Memory data;
	};

	class ImageSet
	{
	public:
		ImageSet() = default;

		/// <summary>
		/// Non copyable ctor
		/// </summary>
		/// <param name=""></param>
		ImageSet(ImageSet&) = delete;

		/// <summary>
		/// Default move ctor
		/// </summary>
		/// <param name=""></param>
		ImageSet(ImageSet&&) = default;

		/// <summary>
		/// Non copyable assign
		/// </summary>
		/// <param name=""></param>
		/// <returns></returns>
		ImageSet& operator=(ImageSet&) = delete;

		/// <summary>
		/// Default move assign
		/// </summary>
		/// <param name=""></param>
		/// <returns></returns>
		ImageSet& operator=(ImageSet&&) = default;

		/// <summary>
		/// Returns the status of the flip image on load flag
		/// </summary>
		/// <returns>true/false</returns>
		bool flipImage() const;

		/// <summary>
		/// Sets the status of the flip image on load flag
		/// </summary>
		/// <param name="aflip">true=should flip</param>
		void setFlipImage(bool aflip);
		
		/// <summary>
		/// Returns the number of image levels
		/// </summary>
		/// <returns>image levels</returns>
		uint levels() const;

		/// <summary>
		/// Returns that this image set is empty (contains zero levels)
		/// </summary>
		/// <returns>true if levels=0, false otherwise</returns>
		bool empty() const;

		/// <summary>
		/// Returns an image at specified level. If level doesn't exist a runtime error generated
		/// </summary>
		/// <param name="level">image level</param>
		/// <returns>const Image reference</returns>
		Image const& at(uint level) const;

		/// <summary>
		/// Returns an image at specified level. If level doesn't exist the operation is undefined
		/// </summary>
		/// <param name="level">image level</param>
		/// <returns>Image reference</returns>
		Image const& operator[](uint level) const;

		/// <summary>
		/// Returns the shape (1D,2D,Cube...) of the undelying image
		/// </summary>
		/// <returns>shape</returns>
		TextureShape shape() const;

		TextureFormat format() const;
		bool srgb() const;

		/// <summary>
		/// Sets the shape of the underlying image set
		/// </summary>
		/// <param name="ashape">TextureShape</param>
		void setShape(TextureShape ashape);

		/// <summary>
		/// Set the format of the image set
		/// </summary>
		/// <param name="aformat">The texture format</param>
		void setFormat(TextureFormat aformat);

		/// <summary>
		/// Creates an ImageSet object with attempt to load the specified image file
		/// </summary>
		/// <param name="afilename">Specifies the filename to load</param>
		/// <returns>Ref to an ImageSet object. If file cannot be load then the ImageSet remains empty</returns>
		ImageSet& fromFile(String const& afilename, bool asrgb = true);

		/// <summary>
		/// Creates an ImageSet object with attempt to load the specified image files
		/// </summary>
		/// <param name="afilename">Specifies the filenames to load</param>
		/// <returns>Ref to an ImageSet object. If file cannot be load then the ImageSet remains empty</returns>
		static Vector<ImageSet> fromFiles(Vector<String> const& afilenames, bool asrgb = true, bool automipmaps = false);

		/// <summary>
		/// Initializes a level of image set with the specified parameters
		/// </summary>
		/// <param name="alevel">level of detail</param>
		/// <param name="awidth">width of the image</param>
		/// <param name="aheight">height of the image</param>
		/// <param name="aformat">pixel format</param>
		/// <param name="asrgb">is SRGB</param>
		/// <param name="adata">raw data</param>
		/// <returns></returns>
		ImageSet& set(ushort alevel, ushort awidth, ushort aheight, TextureFormat aformat, bool asrgb, Memory& adata);

		static ImageSet create(ushort awidth, ushort aheight, TextureFormat aformat, bool asrgb, Memory& adata);

		static ImageSet create(ushort awidth, ushort aheight, TextureFormat aformat);

		void generateMipmaps();

	private:
		TextureShape shape_{};
		Vector<Image> images_{};
		TextureFormat format_{};
		bool srgb_{};
		bool alpha_{};
		bool flip_image_{};
	};

	using CubemapImageSet = Array<ImageSet, 6>;
}
#endif // !GFX_IMAGE_H
