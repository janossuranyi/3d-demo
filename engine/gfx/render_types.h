#pragma once

#include "common.h"
#include "gfx/handle.h"

namespace gfx {

    static const uint16 GLS_CLEAR_COLOR = 0x0001U;
    static const uint16 GLS_CLEAR_DEPTH = 0x0002U;
    static const uint16 GLS_CLEAR_STENCIL = 0x0004U;

    using StateBits = uint64;

    using StorageFlags = uint32_t;
    enum eStorageFlags :uint32_t {
        CREATE_BUFFER_STORAGE_MAP_READ_BIT = 1,
        CREATE_BUFFER_STORAGE_MAP_WRITE_BIT = 2,
        CREATE_BUFFER_STORAGE_MAP_PERSISTENT_BIT = 4,
        CREATE_BUFFER_STORAGE_MAP_COHERENT_BIT = 8,
        CREATE_BUFFER_STORAGE_DYNAMIC_BIT = 16,
    };

    using Result = uint32_t;
    enum eResult { RESULT_SUCCESS = 0, RESULT_FAILED = 255 };

    struct Rect2D { uvec2 offset; uvec2 size; };

    struct BufferTag {};
    struct MaterialTag {};
    struct VertexLayoutTag {};
    struct VertexBufferTag {};
    struct TextureBufferTag {};
    struct IndexBufferTag {};
    struct DynVertexBufferTag {};
    struct DynIndexBufferTag {};
    struct ShaderTag {};
    struct ProgramTag {};
    struct TextureTag {};
    struct FrameBufferTag {};
    struct ConstantBufferTag {};
    struct FenceTag {};
    struct ShaderStorageBufferTag {};
    struct QueryTag {};

    using BufferHandle = Handle<BufferTag, -1>;
    using QueryHandle = Handle<QueryTag, -1>;
    using MaterialHandle = Handle<MaterialTag, -1>;
    using ShaderStorageBufferHandle = Handle<ShaderStorageBufferTag, -1>;
    using TextureBufferHandle = Handle<TextureBufferTag, -1>;
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

    enum class BufferTarget { Vertex, Index, Texture, Uniform, ShaderStorage };

    // alpha mode
    enum class AlphaMode { Opaque, Mask, Blend };

    enum class MaterialWorkflow { MetallicRoughness, SpecularGlossiness };
    // Renderer type
    enum class RendererType { Null, OpenGL };

    // Shader stage
    enum class ShaderStage { Vertex, Geometry, Fragment, Compute };

    // Light
    enum LightType { Directional, Point, Spot };

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
        const uint Uniform = 1 << 0;
        const uint ShaderImageAccess = 1 << 1;
        const uint TextureUpdate = 1 << 2;
        const uint BufferUpdate = 1 << 3;
        const uint ClientMappedBuffer = 1 << 4;
        const uint FrameBuffer = 1 << 5;
        const uint TransformFeedback = 1 << 6;
        const uint AtomicCounter = 1 << 7;
        const uint ShaderStorage = 1 << 8;
        const uint QueryBuffer = 1 << 9;
    }

    namespace binding {
        const uint Diffuse = 0;
        const uint Normal = 1;
        const uint PBR = 2;
        const uint Occlusion = 3;
        const uint Emissive = 4;
    }

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