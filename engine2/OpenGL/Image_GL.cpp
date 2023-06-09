#include <cassert>
#include <cmath>
#include <GL/glew.h>
#include "engine2/Image.h"
#include "engine2/Logger.h"
#include "engine2/FrameBuffer.h"
#include "./RenderBackend_GL.h"
#include "engine2/RenderSystem.h"

namespace jsr {

	
	Image::Image(const std::string& name_)
	{
		name = name_;
		refCount = 0;
		wrapS = IMR_CLAMP_TO_EDGE;
		wrapT = IMR_CLAMP_TO_EDGE;
		magFilter = IFL_LINEAR;
		minFilter = IFL_LINEAR;
		apiObject = -1;
		apiTarget = -1;
		opts = imageOpts_t{};
		GL_CHECK(glGenTextures(1, (GLuint*)&apiObject));
	}

	void Image::Bind()
	{
		if (apiObject == -1) return;
		if (apiTarget == -1) apiTarget = GL_map_textarget(opts.shape);

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
			GL_CHECK(glBindMultiTextureEXT(GL_TEXTURE0 + texunit, apiTarget, apiObject));
		}
	}

	void Image::Unbind()
	{
		GL_CHECK(glBindTexture(GL_TEXTURE_1D, 0));
		GL_CHECK(glBindTexture(GL_TEXTURE_1D_ARRAY, 0));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, 0));
		GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
		GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0));
		for (int i = 0; i < MAX_TEXTURE_UNITS; ++i)
		{
			glcontext.tmu[i].current2DArray = -1;
			glcontext.tmu[i].current2DMap = -1;
			glcontext.tmu[i].currentCubeMap = -1;
			glcontext.tmu[i].currentCubeMapArray = -1;
		}
	}

	int Image::GetId() const
	{
		return id;
	}

	bool Image::IsValid() const
	{
		return id >= 0;
	}

	bool Image::UpdateImageData(int w, int h, int level, int layer, int face, int size, const void* data, eImageFormat srcFormat)
	{
		if (apiObject == -1)
		{
			GL_CHECK(glGenTextures(1, (GLuint*)&apiObject));
		}
		GLenum internalFormat = opts.srgb ? s_image_formats[opts.format].internalFormatSRGB : s_image_formats[opts.format].internalFormat;
		GLenum glformat = s_image_formats[opts.format].format;
		GLenum const gltype = s_image_formats[opts.format].type;
		apiTarget = GL_map_textarget(opts.shape);

		if (srcFormat != IMF_DEFAULT)
		{
			glformat = s_image_formats[srcFormat].format;
		}
		if (opts.autocompress)
		{
			if (!opts.srgb)
			{
				switch (opts.format)
				{
				case IMF_R8:
					internalFormat = GL_COMPRESSED_RED_RGTC1_EXT;
					break;
				case IMF_RG8:
					internalFormat = GL_COMPRESSED_RED_GREEN_RGTC2_EXT;
					break;
				case IMF_RGB:
					internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
					break;
				case IMF_RGBA:
					internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
					break;
				default:
					internalFormat = GL_COMPRESSED_RGBA;
				}
			}
			else
			{
				switch (opts.format)
				{
				case IMF_R8:
					internalFormat = GL_COMPRESSED_RED_RGTC1_EXT;
					break;
				case IMF_RG8:
					internalFormat = GL_COMPRESSED_RED_GREEN_RGTC2_EXT;
					break;
				case IMF_RGB:
					internalFormat = GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
					break;
				case IMF_RGBA:
					internalFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
					break;
				default:
					internalFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
				}
			}
		}

		SetTextureParameters();

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

		if (opts.automipmap && !opts.compressed)
		{
			GL_CHECK(glGenerateMipmap(apiTarget));
		}

		return true;
	}

	void Image::CopyFramebuffer(int x, int y, int imageWidth, int imageHeight)
	{
		if (apiObject == -1)
		{
			GL_CHECK(glGenTextures(1, (GLuint*)&apiObject));
		}

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
			GL_CHECK(glCopyTexImage2D(apiTarget, 0, GL_R11F_G11F_B10F, x, y, imageWidth, imageHeight, 0));
		}
	}

	void Image::SetTextureParameters() const
	{
		const float borderColor[] = { 0.0f,0.0f,0.0f,1.0f };
		//Bind();
		GL_CHECK(glTexParameteri(apiTarget, GL_TEXTURE_WRAP_S, GL_map_texrepeat(wrapS)));
		GL_CHECK(glTexParameteri(apiTarget, GL_TEXTURE_WRAP_T, GL_map_texrepeat(wrapT)));
		GL_CHECK(glTexParameteri(apiTarget, GL_TEXTURE_MIN_FILTER, GL_map_texfilter(minFilter)));
		GL_CHECK(glTexParameteri(apiTarget, GL_TEXTURE_MAG_FILTER, GL_map_texfilter(magFilter)));
		//glTexParameteri(apiTarget, GL_TEXTURE_MAX_LEVEL, numLevel - 1);
		GL_CHECK(glTexParameterf(apiTarget, GL_TEXTURE_MAX_ANISOTROPY, opts.maxAnisotropy));
		//GL_CHECK(glTexParameterf(apiTarget, GL_TEXTURE_MIN_LOD, 0.0f));
		//GL_CHECK(glTexParameterf(apiTarget, GL_TEXTURE_MAX_LOD, (float)(opts.numLevel - 1)));
		GL_CHECK(glTexParameterf(apiTarget, GL_TEXTURE_LOD_BIAS, 0.0f));
		GL_CHECK(glTexParameterfv(apiTarget, GL_TEXTURE_BORDER_COLOR, &borderColor[0]));
		if (GetUsage() == IMU_DEPTH)
		{
			GL_CHECK(glTexParameteri(apiTarget, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
			GL_CHECK(glTexParameteri(apiTarget, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL));
		}
	}

	bool Image::AllocImage(const imageOpts_t& opts_, eImageFilter minFilter, eImageRepeat repeat)
	{	
		if (apiObject == -1)
		{
			GL_CHECK(glGenTextures(1, (GLuint*)&apiObject));
		}

		this->opts = opts_;

		apiTarget = GL_map_textarget(opts.shape);

		Bind();

		wrapS = repeat;
		wrapT = repeat;
		this->minFilter = minFilter;
		this->magFilter = IFL_LINEAR;
		if (minFilter == IFL_NEAREST_NEAREST || minFilter == IFL_LINEAR_NEAREST || minFilter == IFL_NEAREST)
		{
			this->magFilter = IFL_NEAREST;
		}

		const GLenum internalFormat = opts.srgb ? s_image_formats[opts.format].internalFormatSRGB : s_image_formats[opts.format].internalFormat;
		const GLenum glformat = s_image_formats[opts.format].format;
		const GLenum gltype = s_image_formats[opts.format].type;
		const int numSides = opts.shape == IMS_CUBEMAP || opts.shape == IMS_2D_ARRAY ? 6 : 1;
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
		if (apiObject > -1)
		{
			GL_CHECK(glDeleteTextures(1, (GLuint*) & apiObject));
			apiObject = -1;
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