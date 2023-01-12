#pragma once
#include <cinttypes>
#include <unordered_map>
#include <iterator>
#include <GL/glew.h>
#include <SDL.h>
#include "gfx/renderer.h"
#include "gfx/render_context.h"

struct ktxTexture;

namespace gfx {


	// GL TextureFormatInfo.
	struct TextureFormatInfo {
		GLenum internal_format;
		GLenum internal_format_srgb;
		GLenum format;
		GLenum type;
		bool supported;
		ushort pixelByteSize;
	};

	extern TextureFormatInfo s_texture_format[];
	extern const std::unordered_map<TextureFilter, GLenum> s_texture_filter_map;
	extern const std::unordered_map<TextureWrap, GLenum> s_texture_wrap_map;


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


	class OpenGLRenderContext : public RenderContext {
	public:
		OpenGLRenderContext() = default;
		~OpenGLRenderContext();

		void set_state(uint64_t stateBits, bool force) override;
		bool create_window(uint16_t w, uint16_t h, bool fullscreen, const std::string& name) override;
		void process_command_list(const std::vector<RenderCommand>& cmds) override;
		void destroy_window() override;
		void start_rendering() override;
		void stop_rendering() override;
		bool frame(const Frame* frame) override;
		ivec2 get_window_size() const override;

		int red_bits() const override;
		int green_bits() const override;
		int blue_bits() const override;
		int depth_bits() const override;
		int stencil_bits() const override;
		int uniform_offset_aligment() const override;
		void* get_mapped_address(BufferHandle handle) override;

		void operator()(const cmd::CreateBuffer& cmd);
		void operator()(const cmd::CreateBufferTexture& cmd);
		void operator()(const cmd::DeleteVertexLayout& cmd);
		void operator()(const cmd::CreateVertexLayout& cmd);
		void operator()(const cmd::UpdateBuffer& cmd);
		void operator()(const cmd::DeleteBuffer& cmd);
		void operator()(const cmd::CreateShader& cmd);
		void operator()(const cmd::DeleteShader& cmd);
		void operator()(const cmd::CreateProgram& cmd);
		void operator()(const cmd::LinkProgram& cmd);
		void operator()(const cmd::DeleteProgram& cmd);
		void operator()(const cmd::CreateTexture1D& cmd);
		void operator()(const cmd::CreateTexture2D& cmd);
		void operator()(const cmd::CreateTexture& cmd);
		void operator()(const cmd::CreateTextureCubeMap& cmd);
		void operator()(const cmd::DeleteTexture& cmd);
		void operator()(const cmd::CreateFramebuffer& cmd);
		void operator()(const cmd::UpdateFramebuffer& cmd);
		void operator()(const cmd::DeleteFramebuffer& cmd);
		void operator()(const cmd::CreateConstantBuffer& cmd);
		void operator()(const cmd::UpdateConstantBuffer& cmd);
		void operator()(const cmd::DeleteConstantBuffer& cmd);
		void operator()(const cmd::CreateFence& cmd);
		void operator()(const cmd::DeleteFence& cmd);
		void operator()(const cmd::CreateTextureBuffer& cmd);
		void operator()(const cmd::UpdateTextureBuffer& cmd);
		void operator()(const cmd::DeleteTextureBuffer& cmd);
		void operator()(const cmd::CreateShaderStorageBuffer& cmd);
		void operator()(const cmd::UpdateShaderStorageBuffer& cmd);
		void operator()(const cmd::DeleteShaderStorageBuffer& cmd);

		template <typename T> void operator()(const T& c) {
			static_assert(!std::is_same<T, T>::value, "Unimplemented RenderCommand");
		}

	private:
		struct Window_t {
			int redBits;
			int greenBits;
			int blueBits;
			int depthBits;
			int alphaBits;
			int stencilBits;

			int w;
			int h;
		};

		struct BufferData {
			GLuint buffer;
			GLenum target;
			uint size;
			void* mapptr;
		};

		struct ConstantBufferData {
			GLuint buffer;
			uint size;
			BufferUsage usage;
			void* mapped_address;
		};

		struct VertexBufferData {
			GLuint buffer;
			uint size;
			BufferUsage usage;
		};

		struct ShaderBufferData {
			GLuint buffer;
			uint size;
			BufferUsage usage;
		};

		struct TextureBufferData {
			GLuint buffer;
			uint size;
			BufferUsage usage;
		};

		struct IndexBufferData {
			GLuint buffer;
			uint size;
			BufferUsage usage;
			IndexBufferType type;
		};

		struct ShaderData {
			GLuint shader;
			std::string source;
			ShaderStage stage;
			bool compiled;
		};

		struct ProgramData {
			GLuint program;
			bool linked;
			HashMap<String, GLint> uniform_location_map;
		};

		struct TextureData {
			GLuint texture;
			GLenum target;
			TextureFormat format;
		};

		struct FrameBufferData {
			GLuint frame_buffer;
			GLuint depth_render_buffer;
			ushort width;
			ushort height;
			Vector<TextureHandle> textures;
		};

		StateBits state_bits_;
		GLfloat polyOfsScale_, polyOfsBias_;
		SDL_Window* windowHandle_;
		SDL_GLContext glcontext_;
		bool valid_;
		int glVersion_;

		Window_t window_;

		HashMap<FenceHandle, GLsync> fence_map_;
		HashMap<TextureHandle, TextureData> texture_map_;
		HashMap<ShaderHandle, ShaderData> shader_map_;
		HashMap<ProgramHandle, ProgramData> program_map_;
		HashMap<ConstantBufferHandle, ConstantBufferData> constant_buffer_map_;
		HashMap<ShaderStorageBufferHandle, ShaderBufferData> shader_buffer_map_;
		HashMap<TextureBufferHandle, TextureBufferData> texture_buffer_map_;
		HashMap<FrameBufferHandle, FrameBufferData> frame_buffer_map_;
		HashMap<VertexLayoutHandle, GLuint> vertex_array_map_;
		HashMap<BufferHandle, BufferData> buffer_data_map_;
		Mutex buffer_data_mutex_;

		Mutex query_result_map_mx_{};


		Set<String> gl_extensions_;

		VertexDecl active_vertex_decl_{};

		GLuint shared_vertex_array_{ 0 };
		GLint gl_max_vertex_attribs_{ 0 };

		GLenum active_ib_type_{ 0 };
		vec4 active_clear_color_{ 0.0f,0.0f,0.0f,1.0f };

		// cache
		BufferHandle active_vb_{};
		Array<VertexBinding, MAX_LAYOUT_BUFFERS> active_vbs_{};
		Array<TextureHandle, MAX_IMAGE_UNITS> active_imagetexes_{};
		Array<TextureHandle, MAX_TEXTURE_SAMPLERS> active_textures_{};
		BufferHandle active_ib_{};
		ProgramHandle active_program_{};
		FrameBufferHandle active_fb_{0};
		VertexLayoutHandle active_vertex_layout_{};

		bool scissor_test_{ false };
		int active_vertex_attribs_{ 0 };
		GLuint temp_;
		GLint gl_uniform_buffer_offset_alignment_;
		GLint gl_texture_buffer_offset_alignment_;
		GLint gl_max_shader_storage_block_size_;
		GLint gl_max_uniform_block_size_;

		void compute(const RenderPass& pass);
		void setup_uniforms(ProgramData& program_data, const UniformMap& uniforms);
		void setup_textures(const TextureBinding* textures, size_t n);

		Vector<GLint> compressedFormats_;
		GLuint create_buffer_real(GLenum target, BufferUsage usage, uint pixelByteSize, const Memory data, uint& actualSize);
		void update_buffer_real(GLenum target, GLuint buffer, uint offset, uint pixelByteSize, const Memory data);
		GLenum KTX_load_texture(const cmd::CreateTexture& cmd, ktxTexture* kTexture, GLuint& texture);

		int max_state_change_{};
		int max_texture_change_{};
		int max_program_change_{};
		int max_vb_change_{};
};
}