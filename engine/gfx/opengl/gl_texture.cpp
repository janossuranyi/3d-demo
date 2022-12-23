
//#define KHRONOS_STATIC
#include <ktx.h>
#include <algorithm>
#include "stb_image_resize.h"
#include "gfx/opengl/gl_context.h"
#include "gfx/opengl/gl_helper.h"

#include "logger.h"

namespace gfx {

	void OpenGLRenderContext::operator()(const cmd::CreateBufferTexture& cmd)
	{
		if (texture_map_.count(cmd.htexture))
		{
			return;
		}

		auto& texBuf = texture_buffer_map_.find(cmd.hbuffer);
		if (texBuf == std::end(texture_buffer_map_))
		{
			return;
		}

		GLuint texture;
		const GLenum target = GL_TEXTURE_BUFFER;
		GL_CHECK(glCreateTextures(target, 1, &texture));
		auto& texinfo = s_texture_format[static_cast<int>(cmd.format)];
		if (cmd.size == 0) {
			glTextureBuffer(texture, texinfo.internal_format, texBuf->second.buffer);
		}
		else {
			assert((cmd.offset & (gl_texture_buffer_offset_alignment_ - 1)) == 0);
			glTextureBufferRange(texture, texinfo.internal_format, texBuf->second.buffer, cmd.offset, cmd.size);
		}

		texture_map_.emplace(cmd.htexture, TextureData{ texture, target, cmd.format });
	}

	void OpenGLRenderContext::operator()(const cmd::CreateTexture1D& cmd)
	{
		if (texture_map_.count(cmd.handle))
		{
			return;
		}

		GLuint texture{};
		GLenum target = GL_TEXTURE_1D;

		GL_CHECK(glCreateTextures(target, 1, &texture));
		auto const& texinfo = s_texture_format[static_cast<size_t>(cmd.format)];

		GL_CHECK(glTextureStorage1D(texture, 0, texinfo.internal_format, cmd.width));
		if (!cmd.data.empty())
		{
			GL_CHECK(glTextureSubImage1D(texture, 0, 0, cmd.width, texinfo.format, texinfo.type, cmd.data.data()));
		}

		const GLenum wrap = s_texture_wrap_map.at(cmd.wrap);
		const GLenum min_filter = s_texture_filter_map.at(cmd.min_filter);
		const GLenum mag_filter = s_texture_filter_map.at(cmd.mag_filter);

		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, min_filter));
		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, mag_filter));
		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_WRAP_S, wrap));
		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_WRAP_T, wrap));


		if (cmd.automipmap)
		{
			GL_CHECK(glGenerateTextureMipmap(texture));
		}

		const TextureData t_data{ texture, target, cmd.format };
		texture_map_.emplace(cmd.handle, t_data);

	}

	void OpenGLRenderContext::operator()(const cmd::DeleteTexture& cmd)
	{
		auto t_data = texture_map_.find(cmd.handle);
		if (t_data != std::end(texture_map_))
		{
			GL_CHECK(glDeleteTextures(1, &t_data->second.texture));
			texture_map_.erase(cmd.handle);
		}

#ifdef _DEBUG
		Info("Texture %d deleted", cmd.handle);
#endif

	}

	void OpenGLRenderContext::operator()(const cmd::CreateTexture2D& cmd)
	{
		if (texture_map_.count(cmd.handle))
		{
			return;
		}

		GLuint texture{};
		GLenum target = GL_TEXTURE_2D;

		uint const levels = cmd.data.levels();

		GL_CHECK(glCreateTextures(target, 1, &texture));
		auto& texinfo = s_texture_format[static_cast<size_t>(cmd.data.format())];

		GL_CHECK(glTextureStorage2D(texture, levels,
			(cmd.data.srgb() ? texinfo.internal_format_srgb : texinfo.internal_format),
			cmd.data[0].width,
			cmd.data[0].height));

		for (uint l = 0; l < levels; ++l)
		{
			Image const& I = cmd.data[l];
			if (I.data.empty()) continue;

			GL_CHECK(glTextureSubImage2D(texture, l, 0, 0, I.width, I.height, texinfo.format, texinfo.type, I.data.data()));
		}

		const GLenum wrap = s_texture_wrap_map.at(cmd.wrap);
		const GLenum min_filter = s_texture_filter_map.at(cmd.min_filter);		
		const GLenum mag_filter = s_texture_filter_map.at(cmd.mag_filter);

		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, min_filter));
		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, mag_filter));
		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_WRAP_S, wrap));
		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_WRAP_T, wrap));
		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_MAX_ANISOTROPY, 16));

		if (cmd.automipmap)
		{
			GL_CHECK(glGenerateTextureMipmap(texture));
		}

		const TextureData t_data{ texture, target, cmd.data.format()};
		texture_map_.emplace(cmd.handle, t_data);
	}

	GLenum OpenGLRenderContext::KTX_load_texture(const cmd::CreateTexture& cmd, ktxTexture* kTexture, GLuint& texture)
	{
		KTX_error_code result{};
		if (ktxTexture_NeedsTranscoding(kTexture))
		{
			ktx_texture_transcode_fmt_e tf;

			switch (cmd.transcode_quality) {
			case 0:
				tf = KTX_TTF_BC1_OR_3;
				break;
			case 1:
				tf = KTX_TTF_BC7_RGBA; //BPTC
				break;
			case 2:
				tf = KTX_TTF_ETC;
				break;
			default:
				tf = KTX_TTF_RGBA32;
			}

			result = ktxTexture2_TranscodeBasis((ktxTexture2*)kTexture, tf, 0);

			// Then use VkUpload or GLUpload to create a texture object on the GPU.
		}

		GLenum target{};
		GLenum glerror{};
		result = ktxTexture_GLUpload(kTexture, &texture, &target, &glerror);
		ktxTexture_Destroy(kTexture);

		if (result != KTX_SUCCESS || glerror != GL_NO_ERROR)
		{
			Error("Error creating KTX texture [%d]", cmd.handle);
			//GL_CHECK(glDeleteTextures(1, &texture));
		}
		return target;
	}

	void OpenGLRenderContext::operator()(const cmd::CreateTexture& cmd)
	{
		if (texture_map_.count(cmd.handle))
		{
			return;
		}

		GLuint texture{};
		GLenum target = GL_TEXTURE_2D;

		ktxTexture* kTexture{};
		KTX_error_code result{};

		const bool srgb = (cmd.flags & 1) == 1;
		const bool automipmap = (cmd.flags & 2) == 2;
		const bool compress = (cmd.flags & 4) == 4;
		uint levels = 1;

		result = ktxTexture_CreateFromNamedFile(cmd.path.c_str(), KTX_TEXTURE_CREATE_NO_FLAGS, &kTexture);
		if (result == KTX_SUCCESS)
		{
			target = KTX_load_texture(cmd, kTexture, texture);
			levels = kTexture->numLevels;
			if (!target) return;
		}
		else
		{
			target = GL_TEXTURE_2D;

			// load with stb_image
			ImageSet S;
			S.fromFile(cmd.path, srgb);
			if (S.empty())
			{
				Error("Cant load image %s", cmd.path.c_str());
				return;
			}
			if (automipmap)
			{
				S.generateMipmaps();
			}
			if (S.shape() == TextureShape::D1)
			{
				target = GL_TEXTURE_1D;
			}

			GL_CHECK(glCreateTextures(target, 1, &texture));
			const auto& texinfo = s_texture_format[static_cast<size_t>(S.format())];
			const auto& compinfo = compress ? s_texture_format[static_cast<size_t>(TextureFormat::RGBA8_COMPRESSED)] : texinfo;

			levels = S.levels();

			if (target == GL_TEXTURE_1D)
			{
				GL_CHECK(glTextureStorage1D(texture, levels,
					(srgb ? compinfo.internal_format_srgb : compinfo.internal_format), S[0].width));
			}
			if (target == GL_TEXTURE_2D)
			{
				GL_CHECK(glTextureStorage2D(texture, levels,
					(srgb ? compinfo.internal_format_srgb : compinfo.internal_format), S[0].width, S[0].height));
			}

			for (int k = 0; k < S.levels(); ++k)
			{
				if (target == GL_TEXTURE_1D)
				{
					GL_CHECK(glTextureSubImage1D(texture, k, 0,
						S[k].width, texinfo.format, texinfo.type, S[k].data.data()));
				}
				else
				{
					GL_CHECK(glTextureSubImage2D(texture, k, 0, 0,
						S[k].width, S[k].height, texinfo.format, texinfo.type, S[k].data.data()));
				}
			}
		}

		TextureWrap t_wrap = cmd.wrap;
		if (target == GL_TEXTURE_CUBE_MAP) {
			t_wrap = TextureWrap::ClampToEdge;
		}
		TextureFilter m_filter = cmd.min_filter;
		if (levels == 1) {
			m_filter = TextureFilter::Linear;
		}

		const GLenum wrap = s_texture_wrap_map.at(t_wrap);
		const GLenum min_filter = s_texture_filter_map.at(m_filter);
		const GLenum mag_filter = s_texture_filter_map.at(cmd.mag_filter);

		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, min_filter));
		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, mag_filter));
		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_WRAP_S, wrap));
		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_WRAP_T, wrap));
		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_WRAP_R, wrap));
		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_MAX_ANISOTROPY, 16));

		const TextureData t_data{ texture, target, TextureFormat::RGBA8 };

		texture_map_.emplace(cmd.handle, t_data);

	}

	void OpenGLRenderContext::operator()(const cmd::CreateTextureCubeMap& cmd)
	{
		if (!cmd.data.size()) return;

		GLuint texture;
		const GLenum target = GL_TEXTURE_CUBE_MAP;
		const auto& texinfo = s_texture_format[static_cast<size_t>(cmd.data[0].format())];
		const auto& compinfo = cmd.compress ? s_texture_format[static_cast<size_t>(TextureFormat::RGBA8_COMPRESSED)] : texinfo;
		const GLenum wrap = s_texture_wrap_map.at(cmd.wrap);
		const GLenum min_filter = s_texture_filter_map.at(cmd.min_filter);
		const GLenum mag_filter = s_texture_filter_map.at(cmd.mag_filter);

		GL_CHECK(glCreateTextures(target, 1, &texture));

		const auto& S = cmd.data[0];
		const auto& I = S[0];
		GL_CHECK(glTextureStorage2D(texture, S.levels(), (S.srgb() ? texinfo.internal_format_srgb : texinfo.internal_format), I.width, I.height));

		// Create an empty cube map
		if (cmd.data.size() == 6)
		{
			// upload texture faces
			for (int f = 0; f < 6; ++f)
			{
				const auto& S = cmd.data[f];
				for (int k = 0; k < S.levels(); ++k)
				{
					const auto& I = S[k];
					GL_CHECK(glTextureSubImage3D(texture, k, 0, 0, f,						
						I.width, I.height, 1, texinfo.format, texinfo.type, I.data.data()));
				}
			}
		}

		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, min_filter));
		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, mag_filter));
		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_WRAP_S, wrap));
		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_WRAP_T, wrap));
		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_WRAP_R, wrap));
		GL_CHECK(glTextureParameteri(texture, GL_TEXTURE_MAX_ANISOTROPY, 16));

		const TextureData t_data{ texture, target, cmd.data[0].format() };
		texture_map_.emplace(cmd.handle, t_data);
	}

}