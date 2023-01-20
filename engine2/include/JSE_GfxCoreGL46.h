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
	bool normalized;
	uint16_t componentCount;
};


class JseGfxCoreGL : public JseGfxCore {
public:
	~JseGfxCoreGL() override;
	JseGfxCoreGL();
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

	virtual JseResult UpdateImageData_impl(const JseImageUploadInfo& imgageUploadInfo) override;

	virtual JseResult DeleteImage_impl(JseImageID imageId) override;

	virtual JseResult CreateGraphicsPipeline_impl(const JseGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo) override;
	
	virtual JseResult BindGraphicsPipeline_impl(JseGrapicsPipelineID pipelineId) override;

	virtual JseResult DeleteGraphicsPipeline_impl(JseGrapicsPipelineID pipelineId) override;

	virtual JseResult CreateShader_impl(const JseShaderCreateInfo& shaderCreateInfo, std::string& errorOutput) override;
	
	virtual JseResult CreateFrameBuffer_impl(const JseFrameBufferCreateInfo& frameBufferCreateInfo) override;

	virtual JseResult DeleteFrameBuffer_impl(JseFrameBufferID framebufferId) override;

	virtual JseResult BeginRenderPass_impl(const JseRenderPassInfo& renderPassInfo) override;

	virtual JseResult EndRenderPass_impl() override;

	virtual void BindVertexBuffers_impl(uint32_t firstBinding, uint32_t bindingCount, const JseBufferID* pBuffers, const JseDeviceSize* pOffsets) override;

	virtual void BindVertexBuffer_impl(uint32_t binding, JseBufferID buffer, JseDeviceSize offsets) override;

	virtual void BindIndexBuffer_impl(JseBufferID buffer, uint32_t offset, JseIndexType type) override;

	virtual JseResult GetDeviceCapabilities_impl(JseDeviceCapabilities& dest) override;

	virtual JseResult SetVSyncInterval_impl(int interval) override;

	virtual JseResult GetSurfaceDimension_impl(JseRect2D& x) override;

	virtual void Shutdown_impl() override;

	void SetRenderState(JseRenderState state, bool force = false);

	void _glBindFramebuffer(GLenum a, GLuint b);
	void _glViewport(GLint x, GLint y, GLsizei w, GLsizei h);

	JseDeviceCapabilities deviceCapabilities_{};

	struct BufferData {
		GLuint buffer;
		GLenum target;
		uint32_t size;
		void* mapptr;
	};

	struct ImageData {
		GLuint texture;
		GLenum target;
		GLenum format;
		GLenum type;
	};

	struct GfxPipelineData {
		GLuint vao;
		GLuint program;
		std::vector<JseVertexInputBindingDescription> binding;
		JseRenderState renderState;
	};

	struct ShaderData {
		GLuint shader;
		JseShaderStage stage;
	};

	struct FrameBufferData {
		GLuint framebuffer;
	};

	struct glStateCache {
		GLint unpackAlignment;
		GLuint framebuffer;
		JseRect2D viewport;
		JseRect2D scissor;
		GLuint indexBuffer;
	} stateCache_{};

	struct ActivePipelineData {
		GfxPipelineData* pData;
	} activePipelineData_;

	JseRenderState gl_state_{};
	GLfloat polyOfsScale_{}, polyOfsBias_{};
	
	std::vector<GLuint> vertex_buffer_bindings_;
	std::vector<GLintptr> vertex_buffer_offsets_;
	std::vector<GLsizei> vertex_buffer_strides_;
	GLintptr active_index_offset_{};
	JseIndexType active_index_type_{};

	JseHashMap<JseBufferID, BufferData> buffer_data_map_;
	JseHashMap<JseImageID, ImageData> texture_data_map_;
	JseHashMap<JseGrapicsPipelineID, GfxPipelineData> pipeline_data_map_;
	JseHashMap<JseShaderID, ShaderData> shader_map_;
	JseHashMap<JseFrameBufferID, FrameBufferData> framebuffer_map_;
};

#endif