#pragma once

#include <new>
#include "common.h"
#include "gfx/handle.h"
#include "gfx/memory.h"
#include "gfx/vertex.h"
#include "gfx/render_types.h"
#include "gfx/image.h"

#define MAX_TEXTURE_SAMPLERS 8
#define MAX_UNIFORM_BUFFERS 8
#define MAX_IMAGE_UNITS 8
#define MAX_LAYOUT_BUFFERS 4
#define MAX_SHADER_STORAGE_BUFFERS 8

namespace gfx {

    struct CreateBufferInfo {
        BufferTarget target;
        StorageFlags flags;
        TextureHandle texture;
        TextureFormat format;
        uint size;
        uint bufferOffset;
        Memory data;
    };

    struct RenderItemDesc {
        BufferHandle vb;
        BufferHandle ib;
        uint vb_offset;
        uint ib_offset;
        uint elements;
    };

    struct FramebufferTexture {
        TextureHandle handle;
        ushort level;
        ushort face;
    };

    namespace cmd {

        struct CreateBuffer {
            BufferHandle handle;
            BufferTarget target;
            StorageFlags flags;
            uint size;
            uint bufferOffset;
            Memory data;
        };

        struct CreateBufferTexture {
            TextureBufferHandle hbuffer;
            TextureHandle htexture;
            TextureFormat format;
            uint offset;
            uint size;
            Memory data;
        };

        struct CreateTextureBuffer {
            TextureBufferHandle handle;
            Memory data;
            uint size;
            BufferUsage usage;
        };
        
        struct UpdateTextureBuffer {
            TextureBufferHandle handle;
            Memory data;
            uint offset;
            uint size;
        };

        struct DeleteTextureBuffer {
            TextureBufferHandle handle;
        };

        struct CreateVertexLayout {
            VertexLayoutHandle handle;
            VertexDecl decl;
        };

        struct DeleteVertexLayout {
            VertexLayoutHandle handle;
        };

        struct CreateShaderStorageBuffer {
            ShaderStorageBufferHandle handle;
            Memory data;
            uint size;
            BufferUsage usage;
        };

        struct UpdateShaderStorageBuffer {
            ShaderStorageBufferHandle handle;
            Memory data;
            uint offset;
            uint size;
        };

        struct DeleteShaderStorageBuffer {
            ShaderStorageBufferHandle handle;
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

        struct UpdateBuffer {
            BufferHandle handle;
            Memory data;
            uint offset;
        };

        struct DeleteBuffer {
            BufferHandle handle;
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
            TextureFilter min_filter;
            TextureFilter mag_filter;
            Memory data;
            bool automipmap;
        };

        struct CreateTexture2D {
            TextureHandle handle;
            TextureWrap wrap;
            TextureFilter min_filter;
            TextureFilter mag_filter;
            ImageSet data;
            bool automipmap;
            ushort max_aniso;
        };

        struct CreateTextureCubeMap {
            TextureHandle handle;
            TextureWrap wrap;
            TextureFilter min_filter;
            TextureFilter mag_filter;
            Vector<ImageSet> data;
            bool compress;
        };

        // flags: 0: srgb, 1: automipmap, 2: compress
        struct CreateTexture {
            TextureHandle handle;
            TextureWrap wrap;
            TextureFilter min_filter;
            TextureFilter mag_filter;
            ushort transcode_quality;
            String path;
            ushort flags;
            ushort max_aniso;
        };

        struct DeleteTexture {
            TextureHandle handle;
        };

        struct CreateFramebuffer {
            FrameBufferHandle handle;
            uint16 width, height;
            TextureHandle depth_stencil_texture;
            Vector<FramebufferTexture> textures;
        };

        struct UpdateFramebuffer {
            FrameBufferHandle handle;
            uint16 width, height;
            Vector<FramebufferTexture> textures;
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
        cmd::CreateBuffer,
        cmd::CreateVertexLayout,
        cmd::DeleteVertexLayout,
        cmd::CreateConstantBuffer,
        cmd::CreateProgram,
        cmd::LinkProgram,
        cmd::CreateFramebuffer,
        cmd::CreateShader,
        cmd::CreateTexture1D,
        cmd::CreateTexture2D,
        cmd::CreateTextureCubeMap,
        cmd::CreateTexture,
        cmd::UpdateConstantBuffer,
        cmd::UpdateBuffer,
        cmd::DeleteConstantBuffer,
        cmd::DeleteFramebuffer,
        cmd::DeleteProgram,
        cmd::DeleteShader,
        cmd::DeleteTexture,
        cmd::DeleteBuffer,
        cmd::CreateFence,
        cmd::DeleteFence,
        cmd::CreateTextureBuffer,
        cmd::UpdateTextureBuffer,
        cmd::UpdateFramebuffer,
        cmd::DeleteTextureBuffer,
        cmd::CreateBufferTexture,
        cmd::CreateShaderStorageBuffer,
        cmd::UpdateShaderStorageBuffer,
        cmd::DeleteShaderStorageBuffer>;

    using VertexAttribData = std::variant<int, uint, float, ivec2, ivec3, ivec4, vec2, vec3, vec4>;
    using UniformData = std::variant<int, float,ivec2, ivec3, ivec4, vec2, vec3, vec4, mat3, mat4, Vector<float>, Vector<vec4>>;
    using UniformMap = HashMap<String, UniformData>;
    using VertexAttribMap = HashMap<ushort, VertexAttribData>;

    struct TextureBinding {
        TextureHandle handle;
    };

    using TextureBindings = Array<TextureBinding, MAX_TEXTURE_SAMPLERS>;

    struct ImageBinding {
        TextureHandle handle;
        ushort level;
        bool layered;
        ushort layer;
        Access access;
        TextureFormat format;
    };

    struct VertexBinding {
        BufferHandle handle;
        uint offset;

        inline bool operator!=(const VertexBinding& other) const
        {
            return !operator==(other);
        }
        inline bool operator==(const VertexBinding& other) const
        {
            return handle == other.handle && offset == other.offset;
        }
    };

    struct ComputeItem {
        alignas(std::hardware_destructive_interference_size)
        ushort num_groups_x;
        ushort num_groups_y;
        ushort num_groups_z;
        ProgramHandle program;
        FenceHandle fence;
        bool wait_sync_client{false};
        uint64 wait_timeout;
        Array<ImageBinding, MAX_IMAGE_UNITS> images;
        Array<TextureBinding, MAX_TEXTURE_SAMPLERS> textures;
        UniformMap uniforms;
        uint barrier_bits;
    };

    struct ConstantBufferBinding {
        ConstantBufferHandle handle;
        uint offset;
        uint size;
    };

    struct RenderItem {
        alignas(std::hardware_destructive_interference_size)
        Array<VertexBinding, MAX_LAYOUT_BUFFERS> vbs;
        Array<TextureBinding, MAX_TEXTURE_SAMPLERS> textures;
        Array<ConstantBufferBinding, MAX_UNIFORM_BUFFERS> constant_buffers;
        BufferHandle ib;
        uint ib_offset;
        uint vb_offset;
        uint vertex_count;
        uint instance_count{1};
        PrimitiveType primitive_type{ PrimitiveType::Triangles };
        ProgramHandle program;
        VertexDecl vertexDecl;
        UniformMap uniforms;
        VertexAttribMap vertexAttribs;
        IndexBufferType indexType;
        bool scissor{ false };
        ushort scissor_x{ 0 };
        ushort scissor_y{ 0 };
        ushort scissor_w{ 0 };
        ushort scissor_h{ 0 };

        StateBits state_bits{ 0 };
        inline bool operator<(const RenderItem& other) const
        {
            if (program != other.program)
            {
                return program.internal() < other.program.internal();
            }

            return state_bits < other.state_bits;
        }
    };
    
    static_assert(alignof(RenderItem) == std::hardware_destructive_interference_size, "");


    struct ShaderStorageBinding {
        ShaderStorageBufferHandle handle;
        uint offset;
        uint size;
    };

    struct RenderPass {

        RenderPass();

        void clear();
        Rect2D render_area;;
        vec4 clear_color;
        ushort clear_bits;
        FrameBufferHandle frame_buffer;
        Vector<RenderItem> render_items;
        Vector<ComputeItem> compute_items;

        bool isRenderAreaValid() const;
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
        Result                      createBuffer(const CreateBufferInfo& createInfo, BufferHandle& handle);
        bool                        init(RendererType type, uint16_t width, uint16_t height, const std::string& title, bool use_thread);
        glm::ivec2                  getFramebufferSize() const;
        inline uint64               getFrameNum() const { return framenum_; };
        int                         getUniformOffsetAligment() const;
        VertexLayoutHandle          createVertexLayout(VertexDecl& decl);
        BufferHandle                createVertexBuffer(uint size, Memory data);
        TextureBufferHandle         createTextureBuffer(uint size, BufferUsage usage, Memory data);
        BufferHandle                createIndexBuffer(uint size, Memory data);
        ConstantBufferHandle        createConstantBuffer(uint size, BufferUsage usage, Memory data);
        ShaderStorageBufferHandle   createShaderStorageBuffer(uint size, BufferUsage usage, Memory data);
        TextureHandle               createTexture2D(TextureWrap wrap, TextureFilter minfilter, TextureFilter magfilter, ImageSet& data, ushort max_iso = 1);
        TextureHandle               createTexture(TextureWrap wrap, TextureFilter minfilter, TextureFilter magfilter, String const& apath, bool srgb, bool auto_mipmap = false, bool compress = true, ushort aQuality = 1, ushort max_aniso = 1);
        TextureHandle               createTextureCubemap(TextureWrap wrap, TextureFilter minfilter, TextureFilter magfilter, Vector<ImageSet>& data, bool compress = false);
        TextureHandle               createBufferTexture(TextureBufferHandle hbuffer, TextureFormat format, uint offset = 0, uint size = 0);
        FrameBufferHandle           createFrameBuffer(uint16_t width, uint16_t height, TextureFormat format);
        FrameBufferHandle           createFrameBuffer(std::vector<FramebufferTexture>& textures, TextureHandle depth_texture);
        ProgramHandle               createProgram();
        ShaderHandle                createShader(ShaderStage stage, const std::string& source);
        FenceHandle                 createFence();
        
        void                linkProgram(ProgramHandle handle, std::vector<ShaderHandle>& shaders);

        void                updateBuffer(BufferHandle handle, Memory data, uint offset);
        void                updateConstantBuffer(ConstantBufferHandle handle, Memory data, uint offset);
        void                updateShaderStorageBuffer(ShaderStorageBufferHandle handle, Memory data, uint offset);
        void                updateTextureBuffer(TextureBufferHandle handle, Memory data, uint offset);

        void                deleteVertexLayout(VertexLayoutHandle handle);
        void                deleteBuffer(BufferHandle handle);
        void                deleteConstantBuffer(ConstantBufferHandle handle);
        void                deleteShaderStorageBuffer(ShaderStorageBufferHandle handle);
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
        void                setIndexBuffer(BufferHandle handle);
        void                setPrimitiveType(PrimitiveType type);
        void                setInstanceCount(uint count);
        void                setTexture(TextureHandle handle, uint16_t unit = 0);

        void                setViewport(unsigned pass, ushort x, ushort y, ushort width, ushort height);
        void                setClearColor(unsigned pass, const glm::vec4& value);
        void                setClearBits(unsigned pass, uint16_t value);
        void                setFrameBuffer(unsigned pass, FrameBufferHandle handle);

        void                setConstBuffer(uint16_t index, ConstantBufferHandle handle, uint offset = 0, uint size = 0);
        void                setVertexDecl(const VertexDecl& decl);
        void                setVertexAttribs(const VertexAttribMap& attribs);
        void                setUniforms(UniformMap& uniforms);

        void                setVertexBuffer(BufferHandle vb, ushort index = 0, uint offset = 0);
        void                submit(uint pass);
        void                submit(uint pass, ProgramHandle program);
        void                submit(uint pass, ProgramHandle program, uint vertex_count);
        void                submit(uint pass, ProgramHandle program, uint vertex_count, uint vb_offset, uint ib_offset, IndexBufferType idxType);
        bool                frame();
        bool                renderFrame(Frame* frame);
        void                waitForFrameEnd();
        VertexDecl const*   defaultVertexDecl() const;
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
        HandleGenerator<BufferHandle> buffer_handle_;
        HandleGenerator<ShaderHandle> shader_handle_;
        HandleGenerator<ProgramHandle> program_handle_;
        HandleGenerator<TextureHandle> texture_handle_;
        HandleGenerator<FrameBufferHandle> frame_buffer_handle_;
        HandleGenerator<TextureBufferHandle> texture_buffer_handle_;
        HandleGenerator<FenceHandle> fence_handle_;
        HandleGenerator<ShaderStorageBufferHandle> shader_storage_buffer_handle_;
        HandleGenerator<QueryHandle> query_handle_;

        HashMap<BufferHandle, IndexBufferType> index_buffer_types_;

        // Textures.
        struct TextureData {
            uint16_t width;
            uint16_t height;
            TextureFormat format;
        };
        HashMap<TextureHandle, TextureData> texture_data_;

        // Framebuffers.
        HashMap<FrameBufferHandle, Vector<FramebufferTexture>> frame_buffer_textures_;

        bool compute_active_;
        uint64 framenum_;

        Frame frames_[2];
        Frame* submit_;
        Frame* render_;

        // Add a command to the submit thread.
        void submitPreFrameCommand(RenderCommand command);
        void submitPostFrameCommand(RenderCommand command);

        // Renderer.
        UniqePtr<RenderContext> shared_render_context_;

        VertexDecl defaultVertexDecl_;

        // Render thread proc.
        void renderThread();

    };

}
