#include <cassert>
#include <cmath>
#include <GL/glew.h>
#include "./Image.h"
#include "./Logger.h"
#include "./FrameBuffer.h"
#include "./RenderBackend_GL.h"
#include "./RenderSystem.h"

namespace jsr {

	
	Image::Image(const std::string& name_)
	{
		name = name_;
		created = true;
		wrapS = IMR_CLAMP_TO_EDGE;
		wrapT = IMR_CLAMP_TO_EDGE;
		magFilter = IFL_LINEAR;
		minFilter = IFL_LINEAR;
		apiObject = -1;
		apiTarget = -1;
		opts = imageOpts_t{};
		GL_CHECK(glGenTextures(1, (GLuint*)&apiObject));
	}

	void Image::Bind() const
	{
		if (!renderSystem.IsInitialized()) return;

		const int texunit = renderSystem.backend->GetCurrentTextureUnit();
		tmu_t* tmu = &glcontext.tmu[texunit];

		bool dirty = false;

		if (opts.shape == IMS_2D && tmu->current2DMap != apiObject)
		{
			tmu->current2DMap = apiObject;
			dirty = true;
		}
		else if (opts.shape == IMS_2D_ARRAY && tmu->current2DArray != apiObject)
		{
			tmu->current2DArray = apiObject;
			dirty = true;
		} 
		else if (opts.shape == IMS_CUBEMAP && tmu->currentCubeMap != apiObject)
		{
			tmu->currentCubeMap = apiObject;
			dirty = true;
		}
		else if (opts.shape == IMS_CUBEMAP_ARRAY && tmu->currentCubeMapArray != apiObject)
		{
			tmu->currentCubeMapArray = apiObject;
			dirty = true;
		}

		if (dirty)
		{
			GL_CHECK(glBindMultiTextureEXT(GL_TEXTURE0 + texunit, GL_map_textarget(opts.shape), apiObject));
		}
	}

	void Image::Unbind() const
	{
		//if (!created) return;
		//GL_CHECK(glBindTexture(apiTarget, 0));
	}

	bool Image::UpdateImageData(int w, int h, int level, int layer, int face, int size, const void* data, eImageFormat srcFormat)
	{
		if (!created) return false;

		GLenum const internalFormat = opts.srgb ? s_image_formats[opts.format].internalFormatSRGB : s_image_formats[opts.format].internalFormat;
		GLenum glformat = s_image_formats[opts.format].format;
		GLenum const gltype = s_image_formats[opts.format].type;
		apiTarget = GL_map_textarget(opts.shape);

		if (srcFormat != IMF_DEFAULT)
		{
			glformat = s_image_formats[srcFormat].format;
		}

		GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		switch (opts.shape)
		{
		case IMS_2D:
			if (opts.compressed)
			{
				GL_CHECK(glCompressedTexImage2D(apiTarget, level, internalFormat, w, h, 0, size, data));
			}
			else
			{
				GL_CHECK(glTexImage2D(apiTarget, level, internalFormat, w, h, 0, glformat, gltype, data));
			}
			break;
		case IMS_2D_ARRAY:
			if (opts.compressed)
			{
				GL_CHECK(glCompressedTexImage3D(apiTarget, level, internalFormat, w, h, layer, 0, size, data));
			}
			else
			{
				GL_CHECK(glTexImage3D(apiTarget, level, internalFormat, w, h, layer, 0, glformat, gltype, data));
			}			
			break;
		case IMS_CUBEMAP:
			if (opts.compressed)
			{
				GL_CHECK(glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, internalFormat, w, h, 0, size, data));
			}
			else
			{
				GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, internalFormat, w, h, 0, glformat, gltype, data));
			}
			break;
		case IMS_CUBEMAP_ARRAY:
			if (opts.compressed)
			{
				GL_CHECK(glCompressedTexImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, internalFormat, w, h, layer * 6, 0, size, data));
			}
			else
			{
				GL_CHECK(glTexImage3D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, internalFormat, w, h, layer * 6, 0, glformat, gltype, data));
			}
			break;
		default:
			assert(false);
		}

		return true;
	}

	void Image::CopyFramebuffer(int x, int y, int imageWidth, int imageHeight)
	{
		Bind();
		opts.sizeX = imageWidth;
		opts.sizeY = imageHeight;

		if (Framebuffer::IsDefaultFramebufferActive())
		{
			GL_CHECK(glReadBuffer(GL_BACK));
			GL_CHECK(glCopyTexImage2D(apiTarget, 0, GL_RGBA8, x, y, imageWidth, imageHeight, 0));
		}
		else
		{
			GL_CHECK(glCopyTexImage2D(apiTarget, 0, GL_RGBA16F, x, y, imageWidth, imageHeight, 0));
		}
	}

	void Image::SetTextureParameters() const
	{
		if (!created) return;
		const float borderColor[] = { 0.0f,0.0f,0.0f,1.0f };
		//Bind();
		GL_CHECK(glTexParameteri(apiTarget, GL_TEXTURE_WRAP_S, GL_map_texrepeat(wrapS)));
		GL_CHECK(glTexParameteri(apiTarget, GL_TEXTURE_WRAP_T, GL_map_texrepeat(wrapT)));
		GL_CHECK(glTexParameteri(apiTarget, GL_TEXTURE_MIN_FILTER, GL_map_texfilter(minFilter)));
		GL_CHECK(glTexParameteri(apiTarget, GL_TEXTURE_MAG_FILTER, GL_map_texfilter(magFilter)));
		//glTexParameteri(apiTarget, GL_TEXTURE_MAX_LEVEL, numLevel - 1);
		GL_CHECK(glTexParameterf(apiTarget, GL_TEXTURE_MAX_ANISOTROPY, opts.maxAnisotropy));
		GL_CHECK(glTexParameterf(apiTarget, GL_TEXTURE_MIN_LOD, 0.0f));
		GL_CHECK(glTexParameterf(apiTarget, GL_TEXTURE_MAX_LOD, (float)(opts.numLevel - 1)));
		GL_CHECK(glTexParameterf(apiTarget, GL_TEXTURE_LOD_BIAS, 0.0f));
		GL_CHECK(glTexParameterfv(apiTarget, GL_TEXTURE_BORDER_COLOR, &borderColor[0]));

	}

	bool Image::AllocImage(const imageOpts_t& opts_, eImageFilter minFilter, eImageRepeat repeat)
	{
		if (created) {
			PurgeImage();
		}
	
		this->opts = opts_;

		GLuint texture = -1;
		GL_CHECK(glGenTextures(1, &texture));
		if (texture == -1)
		{
			Error("glGenTexture failed!");
			return false;
		}
		created = true;
		apiObject = texture;

		Bind();

		wrapS = repeat;
		wrapT = repeat;
		this->minFilter = minFilter;
		this->magFilter = IFL_LINEAR;
		if (minFilter == IFL_NEAREST_NEAREST || minFilter == IFL_LINEAR_NEAREST)
		{
			this->magFilter = IFL_NEAREST;
		}

		GLenum internalFormat = opts.srgb ? s_image_formats[opts.format].internalFormatSRGB : s_image_formats[opts.format].internalFormat;
		GLenum glformat = s_image_formats[opts.format].format;
		GLenum gltype = s_image_formats[opts.format].type;
		apiTarget = GL_map_textarget(opts.shape);
		int numSides = opts.shape == IMS_CUBEMAP || opts.shape == IMS_2D_ARRAY ? 6 : 1;
		//opts.numLevel = (int)std::max(std::floor(std::log2(w)), std::floor(std::log2(h)));
		//opts.numLayer = 0;

#if 1
		if (!opts.compressed)
		{
			for (int side = 0; side < numSides; ++side)
			{
				int ww = opts.sizeX, hh = opts.sizeY;
				for (int level = 0; level < opts.numLevel; ++level)
				{
					if (this->opts.shape != IMS_CUBEMAP && this->opts.shape != IMS_CUBEMAP_ARRAY)
					{
						UpdateImageData(ww, hh, level, side, 0, 0, nullptr);
					}
					else
					{
						UpdateImageData(ww, hh, level, 0, side, 0, nullptr);
					}
					ww = std::max(ww >> 1, 1);
					hh = std::max(hh >> 1, 1);
				}
			}
		}
		else
		{
			Error("Image::AllocImage - Cannot alloc compressed texture!!");
			PurgeImage();
			return false;
		}
#endif
		SetTextureParameters();

		return true;
	}

	void Image::PurgeImage()
	{
		if (created)
		{
			GL_CHECK(glDeleteTextures(1, (GLuint*) & apiObject));
			apiObject = -1;
			created = false;
			for (int i = 0; i < MAX_TEXTURE_UNITS; ++i)
			{
				glcontext.tmu[i].current2DArray = -1;
				glcontext.tmu[i].current2DMap = -1;
				glcontext.tmu[i].currentCubeMap = -1;
				glcontext.tmu[i].currentCubeMapArray = -1;
			}
		}
	}
}