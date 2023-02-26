#ifndef JSE_GFX_CORE_GL_H
#define JSE_GFX_CORE_GL_H

#include <GL/glew.h>
#include <cinttypes>
#include <set>
#include "SDL.h"

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

namespace js
{
	class GfxCoreGL : public js::GfxCore {
	public:
		~GfxCoreGL() override;
		GfxCoreGL();
	private:
		bool useDebugMode_{};
		SDL_Window* windowHandle_;
		SDL_GLContext glcontext_;
		std::set<std::string> gl_extensions_;
		int glVersion_;

		// Inherited via JseGfxCore
		virtual void* GetMappedBufferPointer_impl(JseBufferID id) override;

		virtual Result Init_impl(bool debugMode) override;

		virtual Result CreateSurface_impl(const JseSurfaceCreateInfo& createSurfaceInfo) override;

		virtual Result CreateBuffer_impl(const JseBufferCreateInfo& createBufferInfo) override;

		virtual Result UpdateBuffer_impl(const JseBufferUpdateInfo& bufferUpdateInfo) override;

		virtual Result DestroyBuffer_impl(JseBufferID bufferId) override;

		virtual Result CreateImage_impl(const JseImageCreateInfo& createImageInfo) override;

		virtual Result UpdateImageData_impl(const JseImageUploadInfo& imgageUploadInfo) override;

		virtual Result DeleteImage_impl(JseImageID imageId) override;

		virtual Result CreateGraphicsPipeline_impl(const JseGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo) override;

		virtual Result BindGraphicsPipeline_impl(JseGrapicsPipelineID pipelineId) override;

		virtual Result DeleteGraphicsPipeline_impl(JseGrapicsPipelineID pipelineId) override;

		virtual Result CreateShader_impl(const JseShaderCreateInfo& shaderCreateInfo, std::string& errorOutput) override;

		virtual Result CreateFrameBuffer_impl(const JseFrameBufferCreateInfo& frameBufferCreateInfo) override;

		virtual Result DeleteFrameBuffer_impl(JseFrameBufferID framebufferId) override;

		virtual Result BeginRenderPass_impl(const JseRenderPassInfo& renderPassInfo) override;

		virtual Result CreateDescriptorSetLayout_impl(const JseDescriptorSetLayoutCreateInfo& cmd) override;

		virtual Result CreateDescriptorSet_impl(const JseDescriptorSetCreateInfo& cmd) override;

		virtual Result BindDescriptorSet_impl(uint32_t firstSet, uint32_t descriptorSetCount, const JseDescriptorSetID* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) override;

		virtual Result EndRenderPass_impl() override;

		virtual Result WriteDescriptorSet_impl(const JseWriteDescriptorSet& cmd) override;
		virtual Result CreateFence_impl(JseFenceID id) override;
		virtual Result DeleteFence_impl(JseFenceID id) override;
		virtual Result WaitSync_impl(JseFenceID id, uint64_t time) override;


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

		virtual Result GetDeviceCapabilities_impl(JseDeviceCapabilities& dest) override;

		virtual Result SetVSyncInterval_impl(int interval) override;

		virtual Result GetSurfaceDimension_impl(glm::ivec2& x) override;

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
			JsHashMap<uint32_t, JseDescriptorSetLayoutBinding> bindings;
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
			JsVector<DescriptorImageData> images;
			JsVector<DescriptorBufferData> buffers;
			JseUniformMap uniforms;
			JsHashMap<JsString, GLint> uniform_location_map;
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

		JsHashMap<JseBufferID, BufferData> buffer_data_map_;
		JsHashMap<JseImageID, ImageData> texture_data_map_;
		JsHashMap<JseGrapicsPipelineID, GfxPipelineData> pipeline_data_map_;
		JsHashMap<JseShaderID, ShaderData> shader_map_;
		JsHashMap<JseFrameBufferID, FrameBufferData> framebuffer_map_;
		JsHashMap<JseDescriptorSetLayoutID, SetLayoutData> set_layout_map_;
		JsHashMap<JseDescriptorSetID, DescriptorSetData> set_data_map_;
		JsHashMap<JseFenceID, GLsync> fence_map_;

		Result UpdateImageData_mutable(const JseImageUploadInfo& imgageUploadInfo, const ImageData& data);
		Result UpdateImageData_immutable(const JseImageUploadInfo& imgageUploadInfo, const ImageData& data);

	};
}
#endif