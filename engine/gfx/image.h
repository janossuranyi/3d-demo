#ifndef GFX_IMAGE_H
#define GFX_IMAGE_H

#include "engine/gfx/render_types.h"
#include "engine/gfx/memory.h"

namespace gfx {
	struct Image
	{
		TextureFormat format;
		ushort width;
		ushort height;
		Memory data;
	};

	class ImageSet
	{
	public:
		ImageSet() = default;

		ImageSet(ImageSet&) = delete;

		void operator=(ImageSet&) = delete;

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

		/// <summary>
		/// Sets the shape of the underlying image set
		/// </summary>
		/// <param name="ashape">TextureShape</param>
		void setShape(TextureShape ashape);

		/// <summary>
		/// Creates an ImageSet object with attempt to load the specified image file
		/// </summary>
		/// <param name="afilename">Specifies the filename to load</param>
		/// <returns>Ref to an ImageSet object. If file cannot be load then the ImageSet remains empty</returns>
		static ImageSet& fromFile(String const& afilename);

		/// <summary>
		/// Creates an ImageSet object with attempt to load the specified KTX1/2 image file
		/// </summary>
		/// <param name="afilename">Specifies the filename to load</param>
		/// <returns>Ref to an ImageSet object. If file cannot be load then the ImageSet remains empty</returns>
		static ImageSet& fromKtxFile(String const& afilename);
	private:
		TextureShape shape_{};
		uint levels_{};
		Vector<Image> images_{};
	};
}
#endif // !GFX_IMAGE_H
