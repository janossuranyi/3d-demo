#ifndef JSE_GFX_CORE_GL_H
#define JSE_GFX_CORE_GL_H

#include <GL/glew.h>

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
	virtual void* GetMappedBufferPointer_impl(JseBufferID id) override;
	
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

	virtual JseResult CreateDescriptorSetLayout_impl(const JseDescriptorSetLayoutCreateInfo& cmd) override;

	virtual JseResult CreateDescriptorSet_impl(const JseDescriptorSetCreateInfo& cmd) override;

	virtual JseResult BindDescriptorSet_impl(uint32_t firstSet, uint32_t descriptorSetCount, const JseDescriptorSetID* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) override;

	virtual JseResult EndRenderPass_impl() override;

	virtual JseResult WriteDescriptorSet_impl(const JseWriteDescriptorSet& cmd) override;
	virtual JseResult CreateFence_impl(JseFenceID id) override;
	virtual JseResult DeleteFence_impl(JseFenceID id) override;
	virtual JseResult WaitSync_impl(JseFenceID id, uint64_t time) override;


	virtual void BindVertexBuffers_impl(uint32_t firstBinding, uint32_t bindingCount, const JseBufferID* pBuffers, const JseDeviceSize* pOffsets) override;

	virtual void BindVertexBuffer_impl(uint32_t binding, JseBufferID buffer, JseDeviceSize offsets) override;

	virtual void BindIndexBuffer_impl(JseBufferID buffer, uint32_t offset, JseIndexType type) override;

	virtual void Draw_impl(JseTopology mode, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;

	virtual void DrawIndexed_impl(JseTopology mode, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override;

	virtual void Viewport_impl(const JseRect2D& x) override;

	virtual void Scissor_impl(const JseRect2D& x) override;

	virtual void BeginRendering_impl() override;

	virtual void EndRendering_impl() override;

	virtual void SwapChainNextImage_impl() override;

	virtual JseResult GetDeviceCapabilities_impl(JseDeviceCapabilities& dest) override;

	virtual JseResult SetVSyncInterval_impl(int interval) override;

	virtual JseResult GetSurfaceDimension_impl(glm::ivec2& x) override;

	virtual void Shutdown_impl() override;

	void SetRenderState(JseRenderState state, bool force = false);
	
	struct DescriptorSetData;
	void SetUniforms(DescriptorSetData& set, const JseUniformMap& uniforms);

	void _glBindFramebuffer(GLenum a, GLuint b);
	void _glViewport(GLint x, GLint y, GLsizei w, GLsizei h);
	void _glScissor(GLint x, GLint y, GLsizei w, GLsizei h);
	void _glScissorEnabled(bool b);


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
		GLenum internal_format;
		GLenum type;
		GLuint buffer;
		bool compressed;
		bool immutable;
	};

	struct SetLayoutData;
	struct GfxPipelineData {
		GLuint vao;
		GLuint program;
		std::vector<JseVertexInputBindingDescription> binding;
		JseRenderState renderState;
		//const SetLayoutData* setLayout;
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
		GLuint program;
		glm::vec4 clearColor;
		float clearDepth;
		int clearStencil;
	} stateCache_{};

	struct ActivePipelineData {
		GfxPipelineData* pData;
	} activePipelineData_;

	struct SetLayoutData {
		JseHashMap<uint32_t, JseDescriptorSetLayoutBinding> bindings;
	};

	struct DescriptorImageData {
		uint32_t binding;
		JseDescriptorType type;
		GLuint texture;
		GLenum target;
		GLint level;
		GLboolean layered;
		GLint layer;
		GLenum access;
		GLenum format;
	};
	struct DescriptorBufferData {
		JseDescriptorType type;
		uint32_t binding;
		GLuint buffer;
		GLintptr offset;
		GLsizeiptr size;
	};

	struct DescriptorSetData {
		JseDescriptorSetLayoutID setLayout;
		const SetLayoutData* pLayoutData;
		JseVector<DescriptorImageData> images;
		JseVector<DescriptorBufferData> buffers;
		JseUniformMap uniforms;
		JseHashMap<JseString, GLint> uniform_location_map;
	};

	JseRenderState gl_state_{};
	GLfloat polyOfsScale_{}, polyOfsBias_{};
	
	std::vector<GLuint>		vertex_buffer_bindings_;
	std::vector<GLintptr>	vertex_buffer_offsets_;
	std::vector<GLsizei>	vertex_buffer_strides_;
	GLintptr				active_index_offset_{};
	GLenum					active_index_type_{};
	JseGrapicsPipelineID	active_pipeline_{};
	bool					scissorEnabled_{};

	JseHashMap<JseBufferID, BufferData> buffer_data_map_;
	JseHashMap<JseImageID, ImageData> texture_data_map_;
	JseHashMap<JseGrapicsPipelineID, GfxPipelineData> pipeline_data_map_;
	JseHashMap<JseShaderID, ShaderData> shader_map_;
	JseHashMap<JseFrameBufferID, FrameBufferData> framebuffer_map_;
	JseHashMap<JseDescriptorSetLayoutID, SetLayoutData> set_layout_map_;
	JseHashMap<JseDescriptorSetID, DescriptorSetData> set_data_map_;
	JseHashMap<JseFenceID, GLsync> fence_map_;

	JseResult UpdateImageData_mutable(const JseImageUploadInfo& imgageUploadInfo, const ImageData& data);
	JseResult UpdateImageData_immutable(const JseImageUploadInfo& imgageUploadInfo, const ImageData& data);

};

#endif