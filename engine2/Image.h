#pragma once

#include <string>

namespace jsr {

	enum eImageShape
	{
		IMS_2D,
		IMS_2D_ARRAY,
		IMS_CUBEMAP,
		IMS_CUBEMAP_ARRAY,
		IMS_3D,
		IMS_COUNT
	};

	enum eImageFormat
	{
		IMF_DEFAULT,
		IMF_A8,
		IMF_R8,
		IMF_R16,
		IMF_R16F,
		IMF_R32,
		IMF_R32F,
		IMF_D24,
		IMF_D24S8,
		IMF_D32,
		IMF_D32F,
		IMF_RG8,
		IMF_RG16,
		IMF_RG16F,
		IMF_RGB,
		IMF_RGBA,
		IMF_RGB16F,
		IMF_RGBA16F,
		IMF_RGB32F,
		IMF_RGBA32F,
		IMF_R11G11B10F,
		IMF_RGB_COMPRESSED,
		IMF_RGBA_COMPRESSED,
		IMF_RGB_COMPRESSED_DXT1,
		IMF_RGBA_COMPRESSED_DXT1,
		IMF_RGBA_COMPRESSED_DXT3,
		IMF_RGBA_COMPRESSED_DXT5,
		IMF_RGBA_COMPRESSED_BC7,
	};

	enum eImageUsage
	{
		IMU_DIFFUSE,
		IMU_NORMAL,
		IMU_AORM,		// R:Ambient occlusion, G:Roughness, B:Metalness
		IMU_EMMISIVE,
		IMU_DEPTH,
		IMU_FRAGPOS,
		IMU_HDR,
		IMU_DEFAULT,
		IMU_COUNT
	};

	enum eImageFilter
	{
		IFL_NEAREST,
		IFL_LINEAR,
		IFL_NEAREST_NEAREST,
		IFL_NEAREST_LINEAR,
		IFL_LINEAR_NEAREST,
		IFL_LINEAR_LINEAR
	};
	
	enum eImageRepeat
	{
		IMR_REPEAT,
		IMR_CLAMP_TO_BORDER,
		IMR_CLAMP_TO_EDGE
	};

	struct imageOpts_t
	{
		eImageFormat format;
		eImageShape shape;
		eImageUsage usage;
		float maxAnisotropy;
		int	sizeX;
		int sizeY;
		int numLevel;
		int numLayer;
		bool srgb;
		bool compressed;
		bool autocompress;
		bool automipmap;
		imageOpts_t();
	};

	inline imageOpts_t::imageOpts_t()
	{
		format = IMF_DEFAULT;
		shape = IMS_2D;
		usage = IMU_DEFAULT;
		maxAnisotropy = 1.0f;
		sizeX = 0;
		sizeY = 0;
		numLevel = 0;
		numLayer = 0;
		srgb = false;
		compressed = false;
		autocompress = false;
		automipmap = false;
	}

	class Image
	{
		friend class Framebuffer;
		friend class ImageManager;
	public:
		Image();
		Image(const std::string& name);
		~Image();
		bool AllocImage(const imageOpts_t& opts, eImageFilter minFilter, eImageRepeat repeat);
		void PurgeImage();
		bool Load(const char* filename);
		void Bind();
		static void Unbind();
		int GetId() const;
		bool IsValid() const;
		bool UpdateImageData(int w, int h,  int level, int layer, int face, int size, const void* data, eImageFormat srcFormat = IMF_DEFAULT);
		void CopyFramebuffer(int x, int y, int imageWidth, int imageHeight);
		void SetTextureParameters() const;
		void SetUsage(eImageUsage x);
		void SetFilter(eImageFilter min, eImageFilter mag);
		void SetRepeat(eImageRepeat rS, eImageRepeat  rT);
		void SetNumLevel(int x);
		void SetNumLayer(int x);
		void SetMaxAnisotropy(float f);
		void AddRef();
		void ReleaseRef();
		int GetNumLevel() const;
		int GetNumLayer() const;
		int GetWidth() const;
		int GetHeight() const;
		float GetMaxAnisotropy() const;
		eImageFormat GetFormat() const;
		eImageShape GetShape() const;
		eImageFilter GetMinFilter() const;
		eImageFilter GetMagFilter() const;
		eImageRepeat GetRepeatS() const;
		eImageRepeat GetRepeatT() const;
		eImageUsage GetUsage() const;
		bool IsCreated() const;
		bool IsCompressed() const;
		std::string GetName() const;
		imageOpts_t opts;
	private:
		int id;
		std::string	name;
		int apiObject;
		int apiTarget;
		int refCount;
		eImageFilter magFilter;
		eImageFilter minFilter;
		eImageRepeat wrapS;
		eImageRepeat wrapT;
	};
}