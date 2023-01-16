#ifndef JSE_GFX_CORE_GL_H
#define JSE_GFX_CORE_GL_H

#include <set>
#include <string>
#include <SDL.h>
#include "JSE_Core.h"
#include "JSE_GfxCore.h"

#ifdef _DEBUG
#define GL_FLUSH_ERRORS() while(glGetError() != GL_NO_ERROR) {}
#define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)

#define GLC() do { \
            CheckOpenGLError("::", __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#define GL_FLUSH_ERRORS()
#define GLC()
#endif

// GL TextureFormatInfo.
struct TextureFormatInfo {
	GLenum internal_format;
	GLenum internal_format_srgb;
	GLenum format;
	GLenum type;
	bool supported;
	uint16_t pixelByteSize;
};


class JseGfxCoreGL : public JseGfxCore {
public:
	~JseGfxCoreGL() override;
	JseGfxCoreGL() = default;
private:
	bool useDebugMode_{};
	SDL_Window* windowHandle_;
	SDL_GLContext glcontext_;
	std::set<std::string> gl_extensions_;
	int glVersion_;

	// Inherited via JseGfxCore
	virtual JseResult Init_impl(bool debugMode) override;

	virtual JseResult CreateSurface_impl(const JseSurfaceCreateInfo& createSurfaceInfo) override;

	virtual JseResult CreateBuffer_impl(const JseBufferCreateInfo& createBufferInfo) override;

	virtual JseResult UpdateBuffer_impl(const JseBufferUpdateInfo& bufferUpdateInfo) override;

	virtual JseResult DestroyBuffer_impl(JseBufferID bufferId) override;

	virtual JseResult CreateImage_impl(const JseImageCreateInfo& createImageInfo) override;

	virtual JseResult CreateGraphicsPipeline_impl(const JseGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo) override;

	virtual void Shutdown_impl() override;


	struct BufferData {
		GLuint buffer;
		GLenum target;
		uint32_t size;
		void* mapptr;
	};

	struct ImageData {
		GLuint texture;
		GLenum target;
	};

	JseHashMap<JseBufferID, BufferData> buffer_data_map_;
	JseHashMap<JseImageID, ImageData> texture_data_map_;
};

#endif