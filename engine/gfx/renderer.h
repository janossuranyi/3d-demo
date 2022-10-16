#pragma once

#include <memory>
#include <string>
#include <array>
#include <unordered_map>
#include <vector>
#include <variant>
#include <thread>
#include <mutex>
#include <glm/glm.hpp>
#include "handle.h"
#include "resource/memory.h"

#define MAX_TEXTURE_SAMPLERS 8

namespace gfx {

	struct VertexBufferTag {};
	struct IndexBufferTag {};
	struct DynVertexBufferTag {};
	struct DynIndexBufferTag {};
	struct ShaderTag {};
	struct ProgramTag {};
	struct TextureTag {};
	struct FrameBufferTag {};

	using VertexBufferHandle		= Handle<VertexBufferTag, -1>;
	using IndexBufferHandle			= Handle<IndexBufferTag, -1>;
	using DynVertexBufferHandle		= Handle<DynVertexBufferTag, -1>;
	using DynIndexBufferHandle		= Handle<DynIndexBufferTag, -1>;
	using ShaderHandle				= Handle<ShaderTag, -1>;
	using ProgramHandle				= Handle<ProgramTag, -1>;
	using TextureHandle				= Handle<TextureTag, -1>;
	using FrameBufferHandle			= Handle<FrameBufferTag, -1>;

	// Renderer type
	enum class RendererType { Null, OpenGL };

	// Shader stage
	enum class ShaderStage { Vertex, Geometry, Fragment, Compute };

	// Buffer usage
	enum class BufferUsage {
		Static,		// never modified
		Dynamic,	// modified per frame
		Stream		// modified multiple times per frame
	};

	// Index buffer type
	enum class IndexBufferType { U16, U32 };
	// Texture format.
/*
 * RGBA16S
 * ^   ^ ^
 * |   | +-- [ ]Unorm
 * |   |     [F]loat
 * |   |     [S]norm
 * |   |     [I]nt
 * |   |     [U]int
 * |   +---- Number of bits per component
 * +-------- Components
 */

    enum class TextureFormat {
        // Colour formats.
        A8,
        R8,
        R8I,
        R8U,
        R8S,
        R16,
        R16I,
        R16U,
        R16F,
        R16S,
        R32I,
        R32U,
        R32F,
        RG8,
        RG8I,
        RG8U,
        RG8S,
        RG16,
        RG16I,
        RG16U,
        RG16F,
        RG16S,
        RG32I,
        RG32U,
        RG32F,
        RGB8,
        RGB8I,
        RGB8U,
        RGB8S,
        BGRA8,
        RGBA8,
        RGBA8I,
        RGBA8U,
        RGBA8S,
        RGBA16,
        RGBA16I,
        RGBA16U,
        RGBA16F,
        RGBA16S,
        RGBA32I,
        RGBA32U,
        RGBA32F,
        RGBA4,
        RGB5A1,
        RGB10A2,
        RG11B10F,
        // Depth formats.
        D16,
        D24,
        D24S8,
        D32,
        D16F,
        D24F,
        D32F,
        D0S8,
        Count
    };

    // Render states.
    enum class RenderState { CullFace, Depth, Blending };
    enum class CullFrontFace { CCW, CW };
    enum class PolygonMode { Fill, Wireframe };
    enum class BlendFunc {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
        SrcAlphaSaturate
    };
    enum class BlendEquation { Add, Subtract, ReverseSubtract, Min, Max };

    // texture
    enum class TextureWrap { Repeat, ClampToEdge, ClampToBorder, MirroredRepeat };
    enum class TextureFilter { Nearest, Linear, NearestLinear, LinearNearest };

    // Primitives
    enum class PrimitiveType { Point, Lines, LineStrip, LineLoop, Triangles, TriangleFan, TriangleStrip };

    namespace cmd {
        struct CreateVertexBuffer {
            VertexBufferHandle handle;
            Memory data;
            uint32_t size;
            BufferUsage usage;
        };

        struct UpdateVertexBuffer {
            VertexBufferHandle handle;
            Memory data;
            uint32_t offset;
        };

        struct DeleteVertexBuffer {
            VertexBufferHandle handle;
        };

        struct CreateIndexBuffer {
            IndexBufferHandle handle;
            Memory data;
            uint32_t size;
            BufferUsage usage;
            IndexBufferType type;
        };

        struct UpdateIndexBuffer {
            IndexBufferHandle handle;
            Memory data;
            uint32_t offset;
        };

        struct DeleteIndexBuffer {
            IndexBufferHandle handle;
        };

        struct CreateShader {
            ShaderHandle handle;
            ShaderStage stage;
            std::string source;
        };

        struct DeleteShader {
            ShaderHandle handle;
        };

        struct CreateProgram {
            ProgramHandle handle;
        };

        struct AttachShader {
            ProgramHandle program_handle;
            ShaderHandle shader_handle;
        };

        struct LinkProgram {
            ProgramHandle handle;
        };

        struct DeleteProgram {
            ProgramHandle handle;
        };

        struct CreateTexture1D {
            TextureHandle handle;
            uint16_t width;
            TextureFormat format;
            TextureWrap wrap;
            TextureFilter filter;
            Memory data;
        };

        struct CreateTexture2D {
            TextureHandle handle;
            uint16_t width, height;
            TextureFormat format;
            TextureWrap wrap;
            TextureFilter filter;
            Memory data;
        };

        struct CreateTextureCubeMap {
            TextureHandle handle;
            uint16_t width, height;
            TextureFormat format;
            TextureWrap wrap;
            TextureFilter filter;
            std::array<Memory, 6> data;
        };

        struct DeleteTexture {
            TextureHandle handle;
        };

        struct CreateFramebuffer {
            FrameBufferHandle handle;
            uint16_t width, height;
            std::vector<TextureHandle> textures;
        };

        struct DeleteFramebuffer {
            FrameBufferHandle handle;
        };
    }

    using RenderCommand =
        std::variant<cmd::CreateVertexBuffer,
        cmd::CreateIndexBuffer,
        cmd::CreateProgram,
        cmd::CreateFramebuffer,
        cmd::CreateShader,
        cmd::CreateTexture1D,
        cmd::CreateTexture2D,
        cmd::CreateTextureCubeMap,
        cmd::UpdateIndexBuffer,
        cmd::UpdateVertexBuffer,
        cmd::DeleteFramebuffer,
        cmd::DeleteIndexBuffer,
        cmd::DeleteProgram,
        cmd::DeleteShader,
        cmd::DeleteTexture,
        cmd::DeleteVertexBuffer>;

    using UniformData = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4>;

    /*
    encoded_state
    666655555555554444444444333333333322222222221111111111
    3210987654321098765432109876543210987654321098765432109876543210
                                                            | |||||||___ scissor (1)
                                                            | |||||_____ depth test (1)
                                                            | ||||______ cull face (1)
                                                            | |||_______ 0=CCW,1:CW (1)
                                                            | ||________ 0=Fill,1=Wireframe (1)
                                                            \ /
                                                             |__________ BlendEq Color (3)
    
    
    */  
    struct RenderItem {
        struct TextureBinding {
            TextureHandle handle;
        };

        VertexBufferHandle vb;
        IndexBufferHandle ib;
        size_t ib_offset;
        uint32_t primitive_count;
        PrimitiveType primitive_type;
        ProgramHandle program;
        std::unordered_map<std::string, UniformData> uniforms;
        std::array<TextureBinding, MAX_TEXTURE_SAMPLERS> textures;

        bool scissor = false;
        uint16_t scissor_x = 0;
        uint16_t scissor_y = 0;
        uint16_t scissor_w = 0;
        uint16_t scissor_h = 0;

        bool depth_test = true;
        bool cull_face = true;
        CullFrontFace cull_front_face = CullFrontFace::CCW;
        PolygonMode polygon_mode = PolygonMode::Fill;
        bool blend = false;
        BlendEquation blend_eq_color = BlendEquation::Add;
        BlendEquation blend_eq_alpha = BlendEquation::Add;
        BlendFunc blend_src_color = BlendFunc::One;
        BlendFunc blend_dst_color = BlendFunc::Zero;
        BlendFunc blend_src_alpha = BlendFunc::One;
        BlendFunc blend_dst_alpha = BlendFunc::Zero;
        bool color_mask = true;
        bool depth_mask = true;

        uint64_t encoded_state;
        void encode() {}
    };

    struct View {
        void clear();
        glm::vec4 clear_color{ 0.0f,0.0f,0.0f,1.0f };
        FrameBufferHandle frame_buffer{FrameBufferHandle::invalid};
        std::vector<RenderItem> render_items;
    };

    class Renderer;
    class Frame {
        Frame();
        ~Frame() = default;

        RenderItem active_item;
        std::vector<View> views;
        std::vector<RenderCommand> commands_pre;
        std::vector<RenderCommand> commands_post;
    };

    // Low level renderer.
    class RenderContext;
    class Renderer {
    public:
        Renderer();
        ~Renderer();

        bool init(RendererType type, uint16_t width, uint16_t height, const std::string& title, bool use_thread);

    private:
        uint16_t width_, height_;
        std::string window_title_;

        bool use_thread_;
        std::thread render_thread_;

        // Handles.
        HandleGenerator<VertexBufferHandle> vertex_buffer_handle_;
        HandleGenerator<IndexBufferHandle> index_buffer_handle_;
        HandleGenerator<ShaderHandle> shader_handle_;
        HandleGenerator<ProgramHandle> program_handle_;
        HandleGenerator<TextureHandle> texture_handle_;
        HandleGenerator<FrameBufferHandle> frame_buffer_handle_;

        std::unordered_map<IndexBufferHandle, IndexBufferType> index_buffer_types_;

        // Textures.
        struct TextureData {
            uint16_t width;
            uint16_t height;
            TextureFormat format;
        };
        std::unordered_map<TextureHandle, TextureData> texture_data_;

        // Framebuffers.
        std::unordered_map<FrameBufferHandle, std::vector<TextureHandle>> frame_buffer_textures_;

        std::mutex swap_mutex_;
        std::condition_variable swap_cv_;
        bool swapped_frames_;

        Frame frames_[2];
        Frame* submit_;
        Frame* render_;

        // Add a command to the submit thread.
        void submitPreFrameCommand(RenderCommand command);
        void submitPostFrameCommand(RenderCommand command);

        // Renderer.
        std::unique_ptr<RenderContext> shared_render_context_;

        // Render thread proc.
        void renderThread();
        bool renderFrame(Frame* frame);

    };
}