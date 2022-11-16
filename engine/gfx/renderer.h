#pragma once

#include <new>
#include "common.h"
#include "gfx/handle.h"
#include "gfx/memory.h"
#include "gfx/vertex.h"
#include "gfx/render_types.h"

#define MAX_TEXTURE_SAMPLERS 8
#define MAX_UNIFORM_BUFFERS 8
#define MAX_IMAGE_UNITS 8
#define MAX_LAYOUT_BUFFERS 4
#define MAX_SHADER_STORAGE_BUFFERS 8

namespace gfx {


    namespace cmd {

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
        cmd::DeleteFence,
        cmd::CreateTextureBuffer,
        cmd::UpdateTextureBuffer,
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

    //using TextureBindings = Array<TextureBinding, MAX_TEXTURE_SAMPLERS>;

    struct ImageBinding {
        TextureHandle handle;
        uint16_t level;
        bool layered;
        uint16_t layer;
        Access access;
        TextureFormat format;
    };

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

    struct ComputeItem {
        alignas(std::hardware_destructive_interference_size)
        uint16_t num_groups_x;
        uint16_t num_groups_y;
        uint16_t num_groups_z;
        ProgramHandle program;
        FenceHandle fence;
        bool wait_sync_client{false};
        uint64_t wait_timeout;
        Array<ImageBinding, MAX_IMAGE_UNITS> images;
        Array<TextureBinding, MAX_TEXTURE_SAMPLERS> textures;
        UniformMap const* pUniforms;
        uint barrier_bits;
    };

    
    struct RenderItem {
        alignas(std::hardware_destructive_interference_size)
        Array<VertexBinding, MAX_LAYOUT_BUFFERS> vbs;
        Array<TextureBinding, MAX_TEXTURE_SAMPLERS> textures;
        IndexBufferHandle ib;
        uint ib_offset;
        uint vb_offset;
        uint vertex_count;
        uint instance_count{1};
        PrimitiveType primitive_type;
        ProgramHandle program;
        VertexDecl const* vertexDecl;
        UniformMap const* pUniforms;
        VertexAttribMap const* vertexAttribs;

        bool scissor{ false };
        ushort scissor_x{ 0 };
        ushort scissor_y{ 0 };
        ushort scissor_w{ 0 };
        ushort scissor_h{ 0 };

        StateBits state_bits;
    };
    
    static_assert(alignof(RenderItem) == std::hardware_destructive_interference_size, "");

    struct ConstantBufferBinding {
        ConstantBufferHandle handle;
        uint offset;
        uint size;
    };

    struct ShaderStorageBinding {
        ShaderStorageBufferHandle handle;
        uint offset;
        uint size;
    };

    struct RenderPass {

        RenderPass();

        void clear();

        vec4 clear_color;
        ushort clear_bits;
        FrameBufferHandle frame_buffer;
        std::vector<RenderItem> render_items;
        std::vector<ComputeItem> compute_items;
        std::array<ConstantBufferBinding, MAX_UNIFORM_BUFFERS> constant_buffers;
        std::array<ShaderStorageBinding, MAX_SHADER_STORAGE_BUFFERS> shader_storage_buffers;
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

        bool                        init(RendererType type, uint16_t width, uint16_t height, const std::string& title, bool use_thread);
        glm::ivec2                  getFramebufferSize() const;
        inline uint64               getFrameNum() const { return framenum_; };

        VertexLayoutHandle          createVertexLayout(const VertexDecl& decl);
        VertexBufferHandle          createVertexBuffer(uint size, BufferUsage usage, Memory data);
        TextureBufferHandle         createTextureBuffer(uint size, BufferUsage usage, Memory data);
        IndexBufferHandle           createIndexBuffer(uint size, BufferUsage usage, Memory data);
        ConstantBufferHandle        createConstantBuffer(uint size, BufferUsage usage, Memory data);
        ShaderStorageBufferHandle   createShaderStorageBuffer(uint size, BufferUsage usage, Memory data);
        TextureHandle               createTexture2D(uint16_t width, uint16_t height, TextureFormat format, TextureWrap wrap, TextureFilter minfilter, TextureFilter magfilter, bool srgb, bool mipmap, Memory data);
        TextureHandle               createTextureCubemap(uint16_t width, uint16_t height, TextureFormat format, TextureWrap wrap, TextureFilter minfilter, TextureFilter magfilter, bool srgb, bool mipmap, std::vector<Memory>& data);
        TextureHandle               createBufferTexture(TextureBufferHandle hbuffer, TextureFormat format, uint offset = 0, uint size = 0);
        FrameBufferHandle           createFrameBuffer(uint16_t width, uint16_t height, TextureFormat format);
        FrameBufferHandle           createFrameBuffer(std::vector<TextureHandle>& textures, TextureHandle depth_texture);
        ProgramHandle               createProgram();
        ShaderHandle                createShader(ShaderStage stage, const std::string& source);
        FenceHandle                 createFence();
        
        void                linkProgram(ProgramHandle handle, std::vector<ShaderHandle>& shaders);

        void                updateVertexBuffer(VertexBufferHandle handle, Memory data, uint offset);
        void                updateIndexBuffer(IndexBufferHandle handle, Memory data, uint offset);
        void                updateConstantBuffer(ConstantBufferHandle handle, Memory data, uint offset);
        void                updateShaderStorageBuffer(ShaderStorageBufferHandle handle, Memory data, uint offset);
        void                updateTextureBuffer(TextureBufferHandle handle, Memory data, uint offset);

        void                deleteVertexLayout(VertexLayoutHandle handle);
        void                deleteVertexBuffer(VertexBufferHandle handle);
        void                deleteIndexBuffer(IndexBufferHandle handle);
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
        void                setIndexBuffer(IndexBufferHandle handle);
        void                setPrimitiveType(PrimitiveType type);
        void                setInstanceCount(uint count);
        void                setTexture(TextureHandle handle, uint16_t unit = 0);
        void                setClearColor(unsigned pass, const glm::vec4& value);
        void                setClearBits(unsigned pass, uint16_t value);
        void                setFrameBuffer(unsigned pass, FrameBufferHandle handle);
        void                setConstBuffer(unsigned pass, uint16_t index, ConstantBufferHandle handle);
        void                setVertexDecl(const VertexDecl* decl);
        void                setVertexAttribs(const VertexAttribMap* attribs);
        void                setUniforms(const UniformMap* uniforms);

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
        HandleGenerator<TextureBufferHandle> texture_buffer_handle_;
        HandleGenerator<FenceHandle> fence_handle_;
        HandleGenerator<ShaderStorageBufferHandle> shader_storage_buffer_handle_;

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
        uint64 framenum_;

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

}
