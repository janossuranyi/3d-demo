#pragma once

#include "../common.h"
#include "./handle.h"
#include "./memory.h"
#include "./vertex.h"

#define MAX_TEXTURE_SAMPLERS 8
#define MAX_UNIFORM_BUFFERS 8
#define MAX_IMAGE_UNITS 8
#define MAX_LAYOUT_BUFFERS 4

namespace gfx {

    static const uint16 GLS_CLEAR_COLOR = 0x0001U;
    static const uint16 GLS_CLEAR_DEPTH = 0x0002U;
    static const uint16 GLS_CLEAR_STENCIL = 0x0004U;

    using StateBits = uint64;

    struct VertexLayoutTag {};
    struct VertexBufferTag {};
    struct IndexBufferTag {};
    struct DynVertexBufferTag {};
    struct DynIndexBufferTag {};
    struct ShaderTag {};
    struct ProgramTag {};
    struct TextureTag {};
    struct FrameBufferTag {};
    struct ConstantBufferTag {};
    struct FenceTag {};

    using VertexLayoutHandle = Handle<VertexLayoutTag, -1>;
    using ConstantBufferHandle = Handle<ConstantBufferTag, -1>;
    using VertexBufferHandle = Handle<VertexBufferTag, -1>;
    using IndexBufferHandle = Handle<IndexBufferTag, -1>;
    using DynVertexBufferHandle = Handle<DynVertexBufferTag, -1>;
    using DynIndexBufferHandle = Handle<DynIndexBufferTag, -1>;
    using ShaderHandle = Handle<ShaderTag, -1>;
    using ProgramHandle = Handle<ProgramTag, -1>;
    using TextureHandle = Handle<TextureTag, -1>;
    using FrameBufferHandle = Handle<FrameBufferTag, -1>;
    using FenceHandle = Handle<FenceTag, -1>;

    // Renderer type
    enum class RendererType { Null, OpenGL };

    // Shader stage
    enum class ShaderStage { Vertex, Geometry, Fragment, Compute };

    enum class ClearBits : uint {
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

    enum class Access { Read, Write, ReadWrite };
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
        RGB8_COMPRESSED,
        RGBA8_COMPRESSED,
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
    
    namespace barrier {
        const uint Uniform              = 1 << 0;
        const uint ShaderImageAccess    = 1 << 1;
        const uint TextureUpdate        = 1 << 2;
        const uint BufferUpdate         = 1 << 3;
        const uint ClientMappedBuffer   = 1 << 4;
        const uint FrameBuffer          = 1 << 5;
        const uint TransformFeedback    = 1 << 6;
        const uint AtomicCounter        = 1 << 7;
        const uint ShaderStorage        = 1 << 8;
        const uint QueryBuffer          = 1 << 9;
    }

    enum class AttributeType { Byte, UByte, Short, UShort, Int, UInt, Half, Float };
    enum class AttributeName {
        Position,
        TexCoord0,
        TexCoord1,
        TexCoord2,
        TexCoord3,
        TexCoord4,
        TexCoord5,
        TexCoord6,
        Normal,
        Tangent,
        Color0,
        Color1,
        Color2,
        Color3,
        Color4,
        Color5,
        Color6
    };

    struct VertexAttribute
    {
        AttributeName name;
        AttributeType type;
        ushort count;
        uint offset;
        bool normalized;
        ushort divisor;
        ushort binding;
        ushort stride;
    };

    using AttributeList = std::vector<VertexAttribute>;

    class VertexDecl
    {
    public:

        VertexDecl() = default;
        ~VertexDecl() = default;

        bool empty() const;
        VertexDecl& begin();
        VertexDecl& end();
        VertexDecl& reset_offset();
        size_t size() const;
        const AttributeList& attributes() const;
        VertexDecl& add(AttributeName name, AttributeType type, ushort count, bool normalized, ushort stride, ushort binding = 0, ushort divisor = 0);
        void setHandle(VertexLayoutHandle handle);
        VertexLayoutHandle handle() const;
    private:
        static ushort getTypeSize(AttributeType type);
        ushort offset_;
        AttributeList attributes_;
        VertexLayoutHandle handle_{};
    };

    namespace cmd {

        struct CreateVertexLayout {
            VertexLayoutHandle handle;
            VertexDecl decl;
        };

        struct DeleteVertexLayout {
            VertexLayoutHandle handle;
        };

        struct CreateConstantBuffer {
            ConstantBufferHandle handle;
            Memory data;
            uint size;
            BufferUsage usage;
        };

        struct UpdateConstantBuffer {
            ConstantBufferHandle handle;
            Memory data;
            uint offset;
            uint size;
        };

        struct DeleteConstantBuffer {
            ConstantBufferHandle handle;
        };

        struct CreateVertexBuffer {
            VertexBufferHandle handle;
            Memory data;
            uint size;
            BufferUsage usage;
        };

        struct UpdateVertexBuffer {
            VertexBufferHandle handle;
            Memory data;
            uint offset;
            uint size;
        };

        struct DeleteVertexBuffer {
            VertexBufferHandle handle;
        };

        struct CreateIndexBuffer {
            IndexBufferHandle handle;
            Memory data;
            uint size;
            BufferUsage usage;
            IndexBufferType type;
        };

        struct UpdateIndexBuffer {
            IndexBufferHandle handle;
            Memory data;
            uint offset;
        };

        struct DeleteIndexBuffer {
            IndexBufferHandle handle;
        };

        struct CreateShader {
            ShaderHandle handle;
            ShaderStage stage;
            String source;
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
            Vector<ShaderHandle> shaders;
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
            bool mipmap;
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
            bool mipmap;
            Vector<Memory> data;
        };

        struct DeleteTexture {
            TextureHandle handle;
        };

        struct CreateFramebuffer {
            FrameBufferHandle handle;
            uint16 width, height;
            TextureHandle depth_stencil_texture;
            Vector<TextureHandle> textures;
        };

        struct DeleteFramebuffer {
            FrameBufferHandle handle;
        };

        struct CreateFence {
            FenceHandle handle;
        };

        struct DeleteFence {
            FenceHandle handle;
        };

        struct WaitSync {
            FenceHandle handle;
            uint64_t timeout;
            bool client;
        };

        struct MemoryBarrier {
            uint barrier_bits;
        };
    }

    using RenderCommand = std::variant<
        cmd::CreateVertexLayout,
        cmd::DeleteVertexLayout,
        cmd::CreateConstantBuffer,
        cmd::CreateVertexBuffer,
        cmd::CreateIndexBuffer,
        cmd::CreateProgram,
        cmd::LinkProgram,
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
        cmd::DeleteVertexBuffer,
        cmd::CreateFence,
        cmd::DeleteFence>;

    using UniformData = std::variant<int, float,ivec2, ivec3, ivec4, vec2, vec3, vec4, mat3, mat4, Vector<float>, Vector<vec4>>;
    using UniformMap = HashMap<String, UniformData>;

    struct TextureBinding {
        TextureHandle handle;
    };

    using TextureBindings = std::array<TextureBinding, MAX_TEXTURE_SAMPLERS>;

    struct ImageBinding {
        TextureHandle handle;
        uint16_t level;
        bool layered;
        uint16_t layer;
        Access access;
        TextureFormat format;
    };

    using ImageBindings = std::array<ImageBinding, MAX_IMAGE_UNITS>;

    struct VertexBinding {
        VertexBufferHandle handle;
        uint offset;
        inline bool operator!=(const VertexBinding& other) const
        {
            return handle != other.handle || offset != other.offset;
        }
        inline bool operator==(const VertexBinding& other) const
        {
            return handle == other.handle && offset == other.offset;
        }
    };

    using VertexBuffers = Array<VertexBinding, MAX_LAYOUT_BUFFERS>;

    struct ComputeItem {
        uint16_t num_groups_x;
        uint16_t num_groups_y;
        uint16_t num_groups_z;
        ProgramHandle program;
        FenceHandle fence;
        bool wait_sync_client{false};
        uint64_t wait_timeout;
        ImageBindings images;
        TextureBindings textures;
        UniformMap uniforms;
        uint barrier_bits;
    };

    struct RenderItem {
        VertexBuffers vbs;
        //VertexBufferHandle vb;
        IndexBufferHandle ib;
        uint ib_offset;
        uint vb_offset;
        uint vertex_count;
        uint instance_count{1};
        PrimitiveType primitive_type;
        ProgramHandle program;
        VertexDecl vertexDecl;
        UniformMap uniforms;
        std::array<TextureBinding, MAX_TEXTURE_SAMPLERS> textures;

        bool scissor = false;
        ushort scissor_x = 0;
        ushort scissor_y = 0;
        ushort scissor_w = 0;
        ushort scissor_h = 0;

        StateBits state_bits;
    };

    struct RenderPass {

        RenderPass();

        void clear();

        glm::vec4 clear_color;
        ushort clear_bits;
        FrameBufferHandle frame_buffer;
        std::vector<RenderItem> render_items;
        std::vector<ComputeItem> compute_items;
        std::array<ConstantBufferHandle, MAX_UNIFORM_BUFFERS> constant_buffers;
    };

    class Renderer;
    class Frame {
    public:
        Frame();
        ~Frame() = default;
        RenderPass& renderPass(uint index);

        RenderItem active_item;
        ComputeItem active_compute;

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

        bool                init(RendererType type, uint16_t width, uint16_t height, const std::string& title, bool use_thread);
        glm::ivec2          getFramebufferSize() const;

        VertexLayoutHandle  createVertexLayout(const VertexDecl& decl);
        VertexBufferHandle  createVertexBuffer(uint size, BufferUsage usage, Memory data);
        IndexBufferHandle   createIndexBuffer(uint size, BufferUsage usage, Memory data);
        ConstantBufferHandle createConstantBuffer(uint size, BufferUsage usage, Memory data);
        TextureHandle       createTexture2D(uint16_t width, uint16_t height, TextureFormat format, TextureWrap wrap, TextureFilter minfilter, TextureFilter magfilter, bool srgb, bool mipmap, Memory data);
        TextureHandle       createTextureCubemap(uint16_t width, uint16_t height, TextureFormat format, TextureWrap wrap, TextureFilter minfilter, TextureFilter magfilter, bool srgb, bool mipmap, std::vector<Memory>& data);
        FrameBufferHandle   createFrameBuffer(uint16_t width, uint16_t height, TextureFormat format);
        FrameBufferHandle   createFrameBuffer(std::vector<TextureHandle>& textures, TextureHandle depth_texture);
        ProgramHandle       createProgram();
        ShaderHandle        createShader(ShaderStage stage, const std::string& source);
        FenceHandle         createFence();
        
        void                linkProgram(ProgramHandle handle, std::vector<ShaderHandle>& shaders);

        void                updateVertexBuffer(VertexBufferHandle handle, Memory data, uint offset);
        void                updateIndexBuffer(IndexBufferHandle handle, Memory data, uint offset);
        void                updateConstantBuffer(ConstantBufferHandle handle, Memory data, uint offset);

        void                deleteVertexLayout(VertexLayoutHandle handle);
        void                deleteVertexBuffer(VertexBufferHandle handle);
        void                deleteIndexBuffer(IndexBufferHandle handle);
        void                deleteConstantBuffer(ConstantBufferHandle handle);
        void                deleteFrameBuffer(FrameBufferHandle handle);
        void                deleteProgram(ProgramHandle handle);
        void                deleteShader(ShaderHandle handle);
        void                deleteTexture(TextureHandle handle);
        void                deleteFence(FenceHandle handle);

        void                beginCompute();
        void                endCompute();

        void                setComputeJob(glm::ivec3 num_groups, FenceHandle fence);
        void                WaitSync(FenceHandle handle, bool client, uint64_t timeout);
        void                MemoryBarrier(uint barrier_bits);
        void                setImageTexture(uint16_t unit, TextureHandle handle, uint16_t level, bool layered, uint16_t layer, Access access, TextureFormat format);
        void                setRenderState(StateBits bits);
        void                setScissorEnable(bool enabled);
        void                setScissor(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
        void                setIndexBuffer(IndexBufferHandle handle);
        void                setPrimitiveType(PrimitiveType type);
        void                setInstanceCount(uint count);
        void                setTexure(uint16_t unit, TextureHandle handle);
        void                setClearColor(unsigned pass, const glm::vec4& value);
        void                setClearBits(unsigned pass, uint16_t value);
        void                setFrameBuffer(unsigned pass, FrameBufferHandle handle);
        void                setConstBuffer(unsigned pass, uint16_t index, ConstantBufferHandle handle);
        void                setVertexDecl(const VertexDecl& decl);

        void                setProgramVar(const std::string& name, int value);
        void                setProgramVar(const std::string& name, float value);
        void                setProgramVar(const std::string& name, const glm::vec2& value);
        void                setProgramVar(const std::string& name, const glm::vec3& value);
        void                setProgramVar(const std::string& name, const glm::vec4& value);
        void                setProgramVar(const std::string& name, const glm::mat3& value);
        void                setProgramVar(const std::string& name, const glm::mat4& value);
        void                setProgramVar(const std::string& name, const std::vector<float>& value);
        void                setProgramVar(const std::string& name, const std::vector<glm::vec4>& value);
        void                setProgramVar(const std::string& name, UniformData data);

        void                setVertexBuffer(VertexBufferHandle vb, ushort index = 0, uint offset = 0);
        void                submit(uint pass);
        void                submit(uint pass, ProgramHandle program);
        void                submit(uint pass, ProgramHandle program, uint vertex_count);
        void                submit(uint pass, ProgramHandle program, uint vertex_count, uint vb_offset, uint ib_offset);
        bool                frame();
        bool                renderFrame(Frame* frame);
        void                waitForFrameEnd();

    private:
        uint16_t width_, height_;
        std::string window_title_;

        bool use_thread_;
        bool render_done_{ true };
        bool render_job_submitted_{ false };
        bool should_terminate_{ false };

        Thread render_thread_;
        Mutex render_mtx_;
        ConditionVar render_cond_;

        // Handles.
        HandleGenerator<VertexLayoutHandle> vertex_layout_handle_;
        HandleGenerator<ConstantBufferHandle> constant_buffer_handle_;
        HandleGenerator<VertexBufferHandle> vertex_buffer_handle_;
        HandleGenerator<IndexBufferHandle> index_buffer_handle_;
        HandleGenerator<ShaderHandle> shader_handle_;
        HandleGenerator<ProgramHandle> program_handle_;
        HandleGenerator<TextureHandle> texture_handle_;
        HandleGenerator<FrameBufferHandle> frame_buffer_handle_;
        HandleGenerator<FenceHandle> fence_handle_;

        HashMap<IndexBufferHandle, IndexBufferType> index_buffer_types_;

        // Textures.
        struct TextureData {
            uint16_t width;
            uint16_t height;
            TextureFormat format;
        };
        HashMap<TextureHandle, TextureData> texture_data_;

        // Framebuffers.
        HashMap<FrameBufferHandle, Vector<TextureHandle>> frame_buffer_textures_;

        bool compute_active_;

        Frame frames_[2];
        Frame* submit_;
        Frame* render_;

        // Add a command to the submit thread.
        void submitPreFrameCommand(RenderCommand command);
        void submitPostFrameCommand(RenderCommand command);

        // Renderer.
        UniqePtr<RenderContext> shared_render_context_;

        // Render thread proc.
        void renderThread();

    };

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
