#pragma once
#include <cinttypes>
#include <unordered_map>
#include <GL/glew.h>
#include <SDL.h>
#include "renderer.h"
#include "render_context.h"

namespace gfx {

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
		glm::ivec2 get_window_size() const override;

		int red_bits() const override;
		int green_bits() const override;
		int blue_bits() const override;
		int depth_bits() const override;
		int stencil_bits() const override;

		void operator()(const cmd::DeleteVertexLayout& cmd);
		void operator()(const cmd::CreateVertexLayout& cmd);
		void operator()(const cmd::CreateVertexBuffer& cmd);
		void operator()(const cmd::UpdateVertexBuffer& cmd);
		void operator()(const cmd::DeleteVertexBuffer& cmd);
		void operator()(const cmd::CreateIndexBuffer& cmd);
		void operator()(const cmd::UpdateIndexBuffer& cmd);
		void operator()(const cmd::DeleteIndexBuffer& cmd);
		void operator()(const cmd::CreateShader& cmd);
		void operator()(const cmd::DeleteShader& cmd);
		void operator()(const cmd::CreateProgram& cmd);
		void operator()(const cmd::LinkProgram& cmd);
		void operator()(const cmd::DeleteProgram& cmd);
		void operator()(const cmd::CreateTexture1D& cmd);
		void operator()(const cmd::CreateTexture2D& cmd);
		void operator()(const cmd::CreateTextureCubeMap& cmd);
		void operator()(const cmd::DeleteTexture& cmd);
		void operator()(const cmd::CreateFramebuffer& cmd);
		void operator()(const cmd::DeleteFramebuffer& cmd);
		void operator()(const cmd::CreateConstantBuffer& cmd);
		void operator()(const cmd::UpdateConstantBuffer& cmd);
		void operator()(const cmd::DeleteConstantBuffer& cmd);
		void operator()(const cmd::CreateFence& cmd);
		void operator()(const cmd::DeleteFence& cmd);

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

		struct ConstantBufferData {
			GLuint buffer;
			uint32_t size;
			BufferUsage usage;
		};

		struct VertexBufferData {
			GLuint buffer;
			uint32_t size;
			BufferUsage usage;
		};

		struct IndexBufferData {
			GLuint buffer;
			uint32_t size;
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
			std::unordered_map<std::string, GLint> uniform_location_map;
		};

		struct TextureData {
			GLuint texture;
			GLenum target;
			TextureFormat format;
		};

		struct FrameBufferData {
			GLuint frame_buffer;
			GLuint depth_render_buffer;
			uint16_t width;
			uint16_t height;
			std::vector<TextureHandle> textures;
		};

		StateBits state_bits_;
		GLfloat polyOfsScale_, polyOfsBias_;
		SDL_Window* windowHandle_;
		SDL_GLContext glcontext_;
		bool valid_;
		int glVersion_;

		Window_t window_;

		std::unordered_map<FenceHandle, GLsync> fence_map_;
		std::unordered_map<TextureHandle, TextureData> texture_map_;
		std::unordered_map<ShaderHandle, ShaderData> shader_map_;
		std::unordered_map<ProgramHandle, ProgramData> program_map_;
		std::unordered_map<ConstantBufferHandle, ConstantBufferData> constant_buffer_map_;
		std::unordered_map<VertexBufferHandle, VertexBufferData> vertex_buffer_map_;
		std::unordered_map<IndexBufferHandle, IndexBufferData> index_buffer_map_;
		std::unordered_map<FrameBufferHandle, FrameBufferData> frame_buffer_map_;
		std::unordered_map<VertexLayoutHandle, GLuint> vertex_array_map_;

		VertexDecl active_vertex_decl_{};

		GLuint shared_vertex_array_{ 0 };
		GLint gl_max_vertex_attribs_{ 0 };

		GLenum active_ib_type_{ 0 };

		// cache
		VertexBufferHandle active_vb_{};
		IndexBufferHandle active_ib_{};
		ProgramHandle active_program_{};
		FrameBufferHandle active_fb_{};
		VertexLayoutHandle active_vertex_layout_{};

		bool scissor_test_{ false };
		int active_vertex_attribs_{ 0 };
		GLuint temp_;

		void compute(const RenderPass& pass);
		void setup_uniforms(ProgramData& program_data, const UniformMap& uniforms);
		void setup_textures(const TextureBindings& textures);
	};

}