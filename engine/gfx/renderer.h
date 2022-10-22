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

#define MAX_TEXTURE_SAMPLERS 15
#define MAX_UNIFORM_BUFFERS 8

namespace gfx {
    using StateBits = uint64_t;

    struct VertexBufferTag {};
    struct IndexBufferTag {};
    struct DynVertexBufferTag {};
    struct DynIndexBufferTag {};
    struct ShaderTag {};
    struct ProgramTag {};
    struct TextureTag {};
    struct FrameBufferTag {};
    struct ConstantBufferTag {};

    using ConstantBufferHandle = Handle<ConstantBufferTag, -1>;
    using VertexBufferHandle = Handle<VertexBufferTag, -1>;
    using IndexBufferHandle = Handle<IndexBufferTag, -1>;
    using DynVertexBufferHandle = Handle<DynVertexBufferTag, -1>;
    using DynIndexBufferHandle = Handle<DynIndexBufferTag, -1>;
    using ShaderHandle = Handle<ShaderTag, -1>;
    using ProgramHandle = Handle<ProgramTag, -1>;
    using TextureHandle = Handle<TextureTag, -1>;
    using FrameBufferHandle = Handle<FrameBufferTag, -1>;

    // Renderer type
    enum class RendererType { Null, OpenGL };

    // Shader stage
    enum class ShaderStage { Vertex, Geometry, Fragment, Compute };

    enum class ClearBits : uint32_t {
        Color = 0x01,
        Depth = 0x02,
        Stencil = 0x04
    };

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
    enum class TextureFilter { Nearest, Linear, NearestLinear, LinearNearest, LinearLinear };
    enum class TextureShape { D1, D2, CubeMap, D3 };

    // Primitives
    enum class PrimitiveType { Point, Lines, LineStrip, LineLoop, Triangles, TriangleFan, TriangleStrip };
    

    namespace cmd {
        struct CreateConstantBuffer {
            ConstantBufferHandle handle;
            Memory data;
            uint32_t size;
            BufferUsage usage;
        };

        struct UpdateConstantBuffer {
            ConstantBufferHandle handle;
            Memory data;
            uint32_t offset;
            uint32_t size;
        };

        struct DeleteConstantBuffer {
            ConstantBufferHandle handle;
        };

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
            uint32_t size;
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
            std::vector<ShaderHandle> shaders;
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
            TextureFilter min_filter;
            TextureFilter mag_filter;
            bool srgb;
            Memory data;
        };

        struct CreateTextureCubeMap {
            TextureHandle handle;
            uint16_t width, height;
            TextureFormat format;
            TextureWrap wrap;
            TextureFilter min_filter;
            TextureFilter mag_filter;
            bool srgb;
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

    using RenderCommand = std::variant<
        cmd::CreateConstantBuffer,
        cmd::CreateVertexBuffer,
        cmd::CreateIndexBuffer,
        cmd::CreateProgram,
        cmd::CreateFramebuffer,
        cmd::CreateShader,
        cmd::CreateTexture1D,
        cmd::CreateTexture2D,
        cmd::CreateTextureCubeMap,
        cmd::UpdateConstantBuffer,
        cmd::UpdateIndexBuffer,
        cmd::UpdateVertexBuffer,
        cmd::DeleteConstantBuffer,
        cmd::DeleteFramebuffer,
        cmd::DeleteIndexBuffer,
        cmd::DeleteProgram,
        cmd::DeleteShader,
        cmd::DeleteTexture,
        cmd::DeleteVertexBuffer>;

    using UniformData = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4>;


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

        StateBits state_bits;
    };

    struct RenderPass {

        RenderPass() : 
            clear_color{ 0.0f,0.0f,0.0f,1.0f }, 
            clear_bits{ GLS_CLEAR_COLOR | GLS_CLEAR_DEPTH },
            frame_buffer{ FrameBufferHandle::invalid } {
        }

        void clear();

        glm::vec4 clear_color;
        uint16_t clear_bits;
        FrameBufferHandle frame_buffer;
        std::vector<RenderItem> render_items;
        std::array<ConstantBufferHandle, MAX_UNIFORM_BUFFERS> constant_buffers;
    };

    class Renderer;
    class Frame {
    public:
        Frame();
        ~Frame() = default;

        RenderPass& renderPass(uint32_t index);

        RenderItem active_item;
        std::vector<RenderPass> render_passes;
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
        void renderThread() {};
        bool renderFrame(Frame* frame);

    };

#define uint64 uint64_t

    static const uint16_t GLS_CLEAR_COLOR = 0x0001U;
    static const uint16_t GLS_CLEAR_DEPTH = 0x0002U;
    static const uint16_t GLS_CLEAR_STENCIL = 0x0004U;

    // one/zero is flipped on src/dest so a gl state of 0 is SRC_ONE,DST_ZERO
    static const uint64 GLS_SRCBLEND_ONE = 0 << 0;
    static const uint64 GLS_SRCBLEND_ZERO = 1 << 0;
    static const uint64 GLS_SRCBLEND_DST_COLOR = 2 << 0;
    static const uint64 GLS_SRCBLEND_ONE_MINUS_DST_COLOR = 3 << 0;
    static const uint64 GLS_SRCBLEND_SRC_ALPHA = 4 << 0;
    static const uint64 GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA = 5 << 0;
    static const uint64 GLS_SRCBLEND_DST_ALPHA = 6 << 0;
    static const uint64 GLS_SRCBLEND_ONE_MINUS_DST_ALPHA = 7 << 0;
    static const uint64 GLS_SRCBLEND_BITS = 7 << 0;

    static const uint64 GLS_DSTBLEND_ZERO = 0 << 3;
    static const uint64 GLS_DSTBLEND_ONE = 1 << 3;
    static const uint64 GLS_DSTBLEND_SRC_COLOR = 2 << 3;
    static const uint64 GLS_DSTBLEND_ONE_MINUS_SRC_COLOR = 3 << 3;
    static const uint64 GLS_DSTBLEND_SRC_ALPHA = 4 << 3;
    static const uint64 GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA = 5 << 3;
    static const uint64 GLS_DSTBLEND_DST_ALPHA = 6 << 3;
    static const uint64 GLS_DSTBLEND_ONE_MINUS_DST_ALPHA = 7 << 3;
    static const uint64 GLS_DSTBLEND_BITS = 7 << 3;

    //------------------------
    // these masks are the inverse, meaning when set the glColorMask value will be 0,
    // preventing that channel from being written
    //------------------------
    static const uint64 GLS_DEPTHMASK = 1 << 6;
    static const uint64 GLS_REDMASK = 1 << 7;
    static const uint64 GLS_GREENMASK = 1 << 8;
    static const uint64 GLS_BLUEMASK = 1 << 9;
    static const uint64 GLS_ALPHAMASK = 1 << 10;
    static const uint64 GLS_COLORMASK = (GLS_REDMASK | GLS_GREENMASK | GLS_BLUEMASK);

    static const uint64 GLS_POLYMODE_LINE = 1 << 11;
    static const uint64 GLS_POLYGON_OFFSET = 1 << 12;

    static const uint64 GLS_DEPTHFUNC_LESS = 0 << 13;
    static const uint64 GLS_DEPTHFUNC_ALWAYS = 1 << 13;
    static const uint64 GLS_DEPTHFUNC_GREATER = 2 << 13;
    static const uint64 GLS_DEPTHFUNC_EQUAL = 3 << 13;
    static const uint64 GLS_DEPTHFUNC_BITS = 3 << 13;

    static const uint64 GLS_CULL_FRONTSIDED = 0 << 15;
    static const uint64 GLS_CULL_BACKSIDED = 1 << 15;
    static const uint64 GLS_CULL_TWOSIDED = 2 << 15;
    static const uint64 GLS_CULL_BITS = 2 << 15;
    static const uint64 GLS_CULL_MASK = GLS_CULL_FRONTSIDED | GLS_CULL_BACKSIDED | GLS_CULL_TWOSIDED;

    static const uint64 GLS_BLENDOP_ADD = 0 << 18;
    static const uint64 GLS_BLENDOP_SUB = 1 << 18;
    static const uint64 GLS_BLENDOP_MIN = 2 << 18;
    static const uint64 GLS_BLENDOP_MAX = 3 << 18;
    static const uint64 GLS_BLENDOP_BITS = 3 << 18;

    // stencil bits
    static const uint64 GLS_STENCIL_FUNC_REF_SHIFT = 20;
    static const uint64 GLS_STENCIL_FUNC_REF_BITS = 0xFFll << GLS_STENCIL_FUNC_REF_SHIFT;

    static const uint64 GLS_STENCIL_FUNC_MASK_SHIFT = 28;
    static const uint64 GLS_STENCIL_FUNC_MASK_BITS = 0xFFll << GLS_STENCIL_FUNC_MASK_SHIFT;

#define GLS_STENCIL_MAKE_REF( x ) ( ( (uint64)(x) << GLS_STENCIL_FUNC_REF_SHIFT ) & GLS_STENCIL_FUNC_REF_BITS )
#define GLS_STENCIL_MAKE_MASK( x ) ( ( (uint64)(x) << GLS_STENCIL_FUNC_MASK_SHIFT ) & GLS_STENCIL_FUNC_MASK_BITS )

    // Next 12 bits act as front+back unless GLS_SEPARATE_STENCIL is set, in which case it acts as front.
    static const uint64 GLS_STENCIL_FUNC_ALWAYS = 0ull << 36;
    static const uint64 GLS_STENCIL_FUNC_LESS = 1ull << 36;
    static const uint64 GLS_STENCIL_FUNC_LEQUAL = 2ull << 36;
    static const uint64 GLS_STENCIL_FUNC_GREATER = 3ull << 36;
    static const uint64 GLS_STENCIL_FUNC_GEQUAL = 4ull << 36;
    static const uint64 GLS_STENCIL_FUNC_EQUAL = 5ull << 36;
    static const uint64 GLS_STENCIL_FUNC_NOTEQUAL = 6ull << 36;
    static const uint64 GLS_STENCIL_FUNC_NEVER = 7ull << 36;
    static const uint64 GLS_STENCIL_FUNC_BITS = 7ull << 36;

    static const uint64 GLS_STENCIL_OP_FAIL_KEEP = 0ull << 39;
    static const uint64 GLS_STENCIL_OP_FAIL_ZERO = 1ull << 39;
    static const uint64 GLS_STENCIL_OP_FAIL_REPLACE = 2ull << 39;
    static const uint64 GLS_STENCIL_OP_FAIL_INCR = 3ull << 39;
    static const uint64 GLS_STENCIL_OP_FAIL_DECR = 4ull << 39;
    static const uint64 GLS_STENCIL_OP_FAIL_INVERT = 5ull << 39;
    static const uint64 GLS_STENCIL_OP_FAIL_INCR_WRAP = 6ull << 39;
    static const uint64 GLS_STENCIL_OP_FAIL_DECR_WRAP = 7ull << 39;
    static const uint64 GLS_STENCIL_OP_FAIL_BITS = 7ull << 39;

    static const uint64 GLS_STENCIL_OP_ZFAIL_KEEP = 0ull << 42;
    static const uint64 GLS_STENCIL_OP_ZFAIL_ZERO = 1ull << 42;
    static const uint64 GLS_STENCIL_OP_ZFAIL_REPLACE = 2ull << 42;
    static const uint64 GLS_STENCIL_OP_ZFAIL_INCR = 3ull << 42;
    static const uint64 GLS_STENCIL_OP_ZFAIL_DECR = 4ull << 42;
    static const uint64 GLS_STENCIL_OP_ZFAIL_INVERT = 5ull << 42;
    static const uint64 GLS_STENCIL_OP_ZFAIL_INCR_WRAP = 6ull << 42;
    static const uint64 GLS_STENCIL_OP_ZFAIL_DECR_WRAP = 7ull << 42;
    static const uint64 GLS_STENCIL_OP_ZFAIL_BITS = 7ull << 42;

    static const uint64 GLS_STENCIL_OP_PASS_KEEP = 0ull << 45;
    static const uint64 GLS_STENCIL_OP_PASS_ZERO = 1ull << 45;
    static const uint64 GLS_STENCIL_OP_PASS_REPLACE = 2ull << 45;
    static const uint64 GLS_STENCIL_OP_PASS_INCR = 3ull << 45;
    static const uint64 GLS_STENCIL_OP_PASS_DECR = 4ull << 45;
    static const uint64 GLS_STENCIL_OP_PASS_INVERT = 5ull << 45;
    static const uint64 GLS_STENCIL_OP_PASS_INCR_WRAP = 6ull << 45;
    static const uint64 GLS_STENCIL_OP_PASS_DECR_WRAP = 7ull << 45;
    static const uint64 GLS_STENCIL_OP_PASS_BITS = 7ull << 45;

    // Next 12 bits act as back and are only active when GLS_SEPARATE_STENCIL is set.
    static const uint64 GLS_BACK_STENCIL_FUNC_ALWAYS = 0ull << 48;
    static const uint64 GLS_BACK_STENCIL_FUNC_LESS = 1ull << 48;
    static const uint64 GLS_BACK_STENCIL_FUNC_LEQUAL = 2ull << 48;
    static const uint64 GLS_BACK_STENCIL_FUNC_GREATER = 3ull << 48;
    static const uint64 GLS_BACK_STENCIL_FUNC_GEQUAL = 4ull << 48;
    static const uint64 GLS_BACK_STENCIL_FUNC_EQUAL = 5ull << 48;
    static const uint64 GLS_BACK_STENCIL_FUNC_NOTEQUAL = 6ull << 48;
    static const uint64 GLS_BACK_STENCIL_FUNC_NEVER = 7ull << 48;
    static const uint64 GLS_BACK_STENCIL_FUNC_BITS = 7ull << 48;

    static const uint64 GLS_BACK_STENCIL_OP_FAIL_KEEP = 0ull << 51;
    static const uint64 GLS_BACK_STENCIL_OP_FAIL_ZERO = 1ull << 51;
    static const uint64 GLS_BACK_STENCIL_OP_FAIL_REPLACE = 2ull << 51;
    static const uint64 GLS_BACK_STENCIL_OP_FAIL_INCR = 3ull << 51;
    static const uint64 GLS_BACK_STENCIL_OP_FAIL_DECR = 4ull << 51;
    static const uint64 GLS_BACK_STENCIL_OP_FAIL_INVERT = 5ull << 51;
    static const uint64 GLS_BACK_STENCIL_OP_FAIL_INCR_WRAP = 6ull << 51;
    static const uint64 GLS_BACK_STENCIL_OP_FAIL_DECR_WRAP = 7ull << 51;
    static const uint64 GLS_BACK_STENCIL_OP_FAIL_BITS = 7ull << 51;

    static const uint64 GLS_BACK_STENCIL_OP_ZFAIL_KEEP = 0ull << 54;
    static const uint64 GLS_BACK_STENCIL_OP_ZFAIL_ZERO = 1ull << 54;
    static const uint64 GLS_BACK_STENCIL_OP_ZFAIL_REPLACE = 2ull << 54;
    static const uint64 GLS_BACK_STENCIL_OP_ZFAIL_INCR = 3ull << 54;
    static const uint64 GLS_BACK_STENCIL_OP_ZFAIL_DECR = 4ull << 54;
    static const uint64 GLS_BACK_STENCIL_OP_ZFAIL_INVERT = 5ull << 54;
    static const uint64 GLS_BACK_STENCIL_OP_ZFAIL_INCR_WRAP = 6ull << 54;
    static const uint64 GLS_BACK_STENCIL_OP_ZFAIL_DECR_WRAP = 7ull << 54;
    static const uint64 GLS_BACK_STENCIL_OP_ZFAIL_BITS = 7ull << 54;

    static const uint64 GLS_BACK_STENCIL_OP_PASS_KEEP = 0ull << 57;
    static const uint64 GLS_BACK_STENCIL_OP_PASS_ZERO = 1ull << 57;
    static const uint64 GLS_BACK_STENCIL_OP_PASS_REPLACE = 2ull << 57;
    static const uint64 GLS_BACK_STENCIL_OP_PASS_INCR = 3ull << 57;
    static const uint64 GLS_BACK_STENCIL_OP_PASS_DECR = 4ull << 57;
    static const uint64 GLS_BACK_STENCIL_OP_PASS_INVERT = 5ull << 57;
    static const uint64 GLS_BACK_STENCIL_OP_PASS_INCR_WRAP = 6ull << 57;
    static const uint64 GLS_BACK_STENCIL_OP_PASS_DECR_WRAP = 7ull << 57;
    static const uint64 GLS_BACK_STENCIL_OP_PASS_BITS = 7ull << 57;

    static const uint64 GLS_SEPARATE_STENCIL = GLS_BACK_STENCIL_OP_FAIL_BITS | GLS_BACK_STENCIL_OP_ZFAIL_BITS | GLS_BACK_STENCIL_OP_PASS_BITS;
    static const uint64 GLS_STENCIL_OP_BITS = GLS_STENCIL_OP_FAIL_BITS | GLS_STENCIL_OP_ZFAIL_BITS | GLS_STENCIL_OP_PASS_BITS | GLS_SEPARATE_STENCIL;
    static const uint64 GLS_STENCIL_FRONT_OPS = GLS_STENCIL_OP_FAIL_BITS | GLS_STENCIL_OP_ZFAIL_BITS | GLS_STENCIL_OP_PASS_BITS;
    static const uint64 GLS_STENCIL_BACK_OPS = GLS_SEPARATE_STENCIL;

    static const uint64 GLS_DEPTH_TEST_MASK = 1ull << 60;
    static const uint64 GLS_CLOCKWISE = 1ull << 61;
    static const uint64 GLS_MIRROR_VIEW = 1ull << 62;
    static const uint64 GLS_OVERRIDE = 1ull << 63;		// override the render prog state

    static const uint64 GLS_KEEP = GLS_DEPTH_TEST_MASK;
    static const uint64 GLS_DEFAULT = 0;

#define STENCIL_SHADOW_TEST_VALUE		128
#define STENCIL_SHADOW_MASK_VALUE		255

}
