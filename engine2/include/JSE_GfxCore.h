#ifndef JSE_GFX_CORE_H
#define JSE_GFX_CORE_H

struct JseBufferTag{};
struct JseSamplerTag {};
struct JseImageTag{};
struct JseVertexInputTag{};
struct JseGraphicsPipelineTag{};

using JseBufferID = JseHandle<JseBufferTag, -1>;
using JseImageID = JseHandle<JseImageTag, -1>;
using JseSamplerID = JseHandle<JseSamplerTag, -1>;
using JseVertexInputID = JseHandle<JseVertexInputTag, -1>;
using JseGrapicsPipelineID = JseHandle<JseGraphicsPipelineTag, -1>;

struct JseColor4f {
    float r, g, b, a;
};

enum class JseCubeMapFace {
    POSITIVE_X,
    NEGATIVE_X,
    POSITIVE_Y,
    NEGATIVE_Y,
    POSITIVE_Z,
    NEGATIVE_Z
};

enum JseBufferStorageFlags {
    JSE_BUFFER_STORAGE_PERSISTENT_BIT = 1,
    JSE_BUFFER_STORAGE_COHERENT_BIT = 2,
    JSE_BUFFER_STORAGE_READ_BIT = 4,
    JSE_BUFFER_STORAGE_WRITE_BIT = 8,
    JSE_BUFFER_STORAGE_DYNAMIC_BIT = 16
};

enum class JseFilter { NEAREST, LINEAR, NEAREST_MIPMAP_NEAREST, NEAREST_MIPMAP_LINEAR, LINEAR_MIPMAP_NEAREST, LINEAR_MIPMAP_LINEAR };
enum class JseImageTiling { REPEAT, CLAMP_TO_EDGE, CLAMP_TO_BORDER, MIRRORED_REPEAT };

enum class JseVertexInputRate {
    VERTEX, INSTANCE
};

enum class JseBufferTarget {
	VERTEX,
	INDEX,
	UNIFORM,
	UNIFORM_DYNAMIC,
	SHADER_STORAGE,
	SHADER_STORAGE_DYNAMIC,
	TEXTURE,
	FEEDBACK
};

enum class JseImageTarget {
	D1,
    D1_ARRAY,
	D2,
    D2_ARRAY,
	D3,
    D3_ARRAY,
	CUBEMAP,
    CUBEMAP_ARRAY,
	BUFFER,
	SHADER
};

enum class JseShaderStage {
    VERTEX,
    FRAGMENT,
    GEOMETRY,
    TESSELATION_CONTROL,
    TESSELATION,
    COMPUTE
};

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
enum class JseFormat {
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

/******************************************
Create info structs
*******************************************/

struct JseSurfaceCreateInfo {
	int width;
	int height;
	int colorBits;
	int depthBits;
	int alphaBits;
	int stencilBits;
    int swapInterval;
	bool fullScreen;
};

struct JseBufferCreateInfo {
    JseBufferID bufferId;
    JseBufferTarget target;
    JseBufferStorageFlags storageFlags;
    JseFormat format;
    uint32_t size;
};

struct JseBufferUpdateInfo {
    JseBufferID bufferId;
    uint32_t offset;
    JseMemory data;
};

struct JseSamplerDescription {
    JseFilter minFilter;
    JseFilter magFilter;
    JseImageTiling tilingS;
    JseImageTiling tilingT;
    JseImageTiling tilingR;
    float lodBias;
    float minLod;
    float maxLod;
    float maxAnisotropy;
    JseColor4f borderColor;
};

struct JseSamplerCreateInfo {
    JseSamplerID samplerId;
    JseSamplerDescription samplerDescription;
};

struct JseImageCreateInfo {
    JseImageID imageId;
    JseImageTarget target;
    JseFormat format;
    JseSamplerDescription* samplerDescription;
    JseSamplerID sampler;
    uint32_t width;
    uint32_t height;    // 2D height or Array size
    uint32_t depth;     // 3D depth or Array size
    uint32_t levelCount;
    bool srgb;
};

struct JseImageUploadInfo {
    JseImageID imageId;
    uint32_t level;
    JseCubeMapFace face;
    uint32_t xoffset;
    uint32_t yoffset;
    uint32_t zoffset;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    JseMemory data;
};

struct JseVertexInputBindingDescription {
    uint32_t binding;
    uint32_t stride;
    JseVertexInputRate inputRate;    
};

struct JseVertexInputAttributeDescription {
    uint32_t bindig;
    uint32_t location;
    uint32_t offset;
    JseFormat format;    
};

struct JseShaderModuleStageDescription {
    JseShaderStage stage;
    const char* source;
};

struct JseModuleDescription {
    uint32_t stageCount;
    JseShaderModuleStageDescription* stages;
};

struct JseGraphicsPipelineCreateInfo {
    JseGrapicsPipelineID graphicsPipelineId;
    uint32_t bindingCount;
    JseVertexInputBindingDescription* bindings;
    uint32_t attributeCount;
    JseVertexInputAttributeDescription* attributes;

};

struct JseDeviceCapabilities {
    const char* renderer;
    const char* rendererVersion;
    int maxTextureImageUnits;
    int maxArrayTextureLayers;
    int maxTextureSize;
    int maxComputeSharedMemorySize;
    int maxUniformBlockSize;
    int maxShaderStorageBlockSize;
    int availableVideoMemory;
};

using JseRenderState = uint64_t;

// one/zero is flipped on src/dest so a gl state of 0 is SRC_ONE,DST_ZERO
static const JseRenderState GLS_SRCBLEND_ONE = 0 << 0;
static const JseRenderState GLS_SRCBLEND_ZERO = 1 << 0;
static const JseRenderState GLS_SRCBLEND_DST_COLOR = 2 << 0;
static const JseRenderState GLS_SRCBLEND_ONE_MINUS_DST_COLOR = 3 << 0;
static const JseRenderState GLS_SRCBLEND_SRC_ALPHA = 4 << 0;
static const JseRenderState GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA = 5 << 0;
static const JseRenderState GLS_SRCBLEND_DST_ALPHA = 6 << 0;
static const JseRenderState GLS_SRCBLEND_ONE_MINUS_DST_ALPHA = 7 << 0;
static const JseRenderState GLS_SRCBLEND_BITS = 7 << 0;

static const JseRenderState GLS_DSTBLEND_ZERO = 0 << 3;
static const JseRenderState GLS_DSTBLEND_ONE = 1 << 3;
static const JseRenderState GLS_DSTBLEND_SRC_COLOR = 2 << 3;
static const JseRenderState GLS_DSTBLEND_ONE_MINUS_SRC_COLOR = 3 << 3;
static const JseRenderState GLS_DSTBLEND_SRC_ALPHA = 4 << 3;
static const JseRenderState GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA = 5 << 3;
static const JseRenderState GLS_DSTBLEND_DST_ALPHA = 6 << 3;
static const JseRenderState GLS_DSTBLEND_ONE_MINUS_DST_ALPHA = 7 << 3;
static const JseRenderState GLS_DSTBLEND_BITS = 7 << 3;

//------------------------
// these masks are the inverse, meaning when set the glColorMask value will be 0,
// preventing that channel from being written
//------------------------
static const JseRenderState GLS_DEPTHMASK = 1 << 6;
static const JseRenderState GLS_REDMASK = 1 << 7;
static const JseRenderState GLS_GREENMASK = 1 << 8;
static const JseRenderState GLS_BLUEMASK = 1 << 9;
static const JseRenderState GLS_ALPHAMASK = 1 << 10;
static const JseRenderState GLS_COLORMASK = (GLS_REDMASK | GLS_GREENMASK | GLS_BLUEMASK);

static const JseRenderState GLS_POLYMODE_LINE = 1 << 11;
static const JseRenderState GLS_POLYGON_OFFSET = 1 << 12;

static const JseRenderState GLS_DEPTHFUNC_LESS = 0 << 13;
static const JseRenderState GLS_DEPTHFUNC_ALWAYS = 1 << 13;
static const JseRenderState GLS_DEPTHFUNC_GREATER = 2 << 13;
static const JseRenderState GLS_DEPTHFUNC_EQUAL = 3 << 13;
static const JseRenderState GLS_DEPTHFUNC_BITS = 3 << 13;

static const JseRenderState GLS_CULL_FRONTSIDED = 0 << 15;
static const JseRenderState GLS_CULL_BACKSIDED = 1 << 15;
static const JseRenderState GLS_CULL_TWOSIDED = 2 << 15;
static const JseRenderState GLS_CULL_BITS = 2 << 15;
static const JseRenderState GLS_CULL_MASK = GLS_CULL_FRONTSIDED | GLS_CULL_BACKSIDED | GLS_CULL_TWOSIDED;

static const JseRenderState GLS_BLENDOP_ADD = 0 << 18;
static const JseRenderState GLS_BLENDOP_SUB = 1 << 18;
static const JseRenderState GLS_BLENDOP_MIN = 2 << 18;
static const JseRenderState GLS_BLENDOP_MAX = 3 << 18;
static const JseRenderState GLS_BLENDOP_BITS = 3 << 18;

// stencil bits
static const JseRenderState GLS_STENCIL_FUNC_REF_SHIFT = 20;
static const JseRenderState GLS_STENCIL_FUNC_REF_BITS = 0xFFll << GLS_STENCIL_FUNC_REF_SHIFT;

static const JseRenderState GLS_STENCIL_FUNC_MASK_SHIFT = 28;
static const JseRenderState GLS_STENCIL_FUNC_MASK_BITS = 0xFFll << GLS_STENCIL_FUNC_MASK_SHIFT;

#define GLS_STENCIL_MAKE_REF( x ) ( ( (JseRenderState)(x) << GLS_STENCIL_FUNC_REF_SHIFT ) & GLS_STENCIL_FUNC_REF_BITS )
#define GLS_STENCIL_MAKE_MASK( x ) ( ( (JseRenderState)(x) << GLS_STENCIL_FUNC_MASK_SHIFT ) & GLS_STENCIL_FUNC_MASK_BITS )

// Next 12 bits act as front+back unless GLS_SEPARATE_STENCIL is set, in which case it acts as front.
static const JseRenderState GLS_STENCIL_FUNC_ALWAYS = 0ull << 36;
static const JseRenderState GLS_STENCIL_FUNC_LESS = 1ull << 36;
static const JseRenderState GLS_STENCIL_FUNC_LEQUAL = 2ull << 36;
static const JseRenderState GLS_STENCIL_FUNC_GREATER = 3ull << 36;
static const JseRenderState GLS_STENCIL_FUNC_GEQUAL = 4ull << 36;
static const JseRenderState GLS_STENCIL_FUNC_EQUAL = 5ull << 36;
static const JseRenderState GLS_STENCIL_FUNC_NOTEQUAL = 6ull << 36;
static const JseRenderState GLS_STENCIL_FUNC_NEVER = 7ull << 36;
static const JseRenderState GLS_STENCIL_FUNC_BITS = 7ull << 36;

static const JseRenderState GLS_STENCIL_OP_FAIL_KEEP = 0ull << 39;
static const JseRenderState GLS_STENCIL_OP_FAIL_ZERO = 1ull << 39;
static const JseRenderState GLS_STENCIL_OP_FAIL_REPLACE = 2ull << 39;
static const JseRenderState GLS_STENCIL_OP_FAIL_INCR = 3ull << 39;
static const JseRenderState GLS_STENCIL_OP_FAIL_DECR = 4ull << 39;
static const JseRenderState GLS_STENCIL_OP_FAIL_INVERT = 5ull << 39;
static const JseRenderState GLS_STENCIL_OP_FAIL_INCR_WRAP = 6ull << 39;
static const JseRenderState GLS_STENCIL_OP_FAIL_DECR_WRAP = 7ull << 39;
static const JseRenderState GLS_STENCIL_OP_FAIL_BITS = 7ull << 39;

static const JseRenderState GLS_STENCIL_OP_ZFAIL_KEEP = 0ull << 42;
static const JseRenderState GLS_STENCIL_OP_ZFAIL_ZERO = 1ull << 42;
static const JseRenderState GLS_STENCIL_OP_ZFAIL_REPLACE = 2ull << 42;
static const JseRenderState GLS_STENCIL_OP_ZFAIL_INCR = 3ull << 42;
static const JseRenderState GLS_STENCIL_OP_ZFAIL_DECR = 4ull << 42;
static const JseRenderState GLS_STENCIL_OP_ZFAIL_INVERT = 5ull << 42;
static const JseRenderState GLS_STENCIL_OP_ZFAIL_INCR_WRAP = 6ull << 42;
static const JseRenderState GLS_STENCIL_OP_ZFAIL_DECR_WRAP = 7ull << 42;
static const JseRenderState GLS_STENCIL_OP_ZFAIL_BITS = 7ull << 42;

static const JseRenderState GLS_STENCIL_OP_PASS_KEEP = 0ull << 45;
static const JseRenderState GLS_STENCIL_OP_PASS_ZERO = 1ull << 45;
static const JseRenderState GLS_STENCIL_OP_PASS_REPLACE = 2ull << 45;
static const JseRenderState GLS_STENCIL_OP_PASS_INCR = 3ull << 45;
static const JseRenderState GLS_STENCIL_OP_PASS_DECR = 4ull << 45;
static const JseRenderState GLS_STENCIL_OP_PASS_INVERT = 5ull << 45;
static const JseRenderState GLS_STENCIL_OP_PASS_INCR_WRAP = 6ull << 45;
static const JseRenderState GLS_STENCIL_OP_PASS_DECR_WRAP = 7ull << 45;
static const JseRenderState GLS_STENCIL_OP_PASS_BITS = 7ull << 45;

// Next 12 bits act as back and are only active when GLS_SEPARATE_STENCIL is set.
static const JseRenderState GLS_BACK_STENCIL_FUNC_ALWAYS = 0ull << 48;
static const JseRenderState GLS_BACK_STENCIL_FUNC_LESS = 1ull << 48;
static const JseRenderState GLS_BACK_STENCIL_FUNC_LEQUAL = 2ull << 48;
static const JseRenderState GLS_BACK_STENCIL_FUNC_GREATER = 3ull << 48;
static const JseRenderState GLS_BACK_STENCIL_FUNC_GEQUAL = 4ull << 48;
static const JseRenderState GLS_BACK_STENCIL_FUNC_EQUAL = 5ull << 48;
static const JseRenderState GLS_BACK_STENCIL_FUNC_NOTEQUAL = 6ull << 48;
static const JseRenderState GLS_BACK_STENCIL_FUNC_NEVER = 7ull << 48;
static const JseRenderState GLS_BACK_STENCIL_FUNC_BITS = 7ull << 48;

static const JseRenderState GLS_BACK_STENCIL_OP_FAIL_KEEP = 0ull << 51;
static const JseRenderState GLS_BACK_STENCIL_OP_FAIL_ZERO = 1ull << 51;
static const JseRenderState GLS_BACK_STENCIL_OP_FAIL_REPLACE = 2ull << 51;
static const JseRenderState GLS_BACK_STENCIL_OP_FAIL_INCR = 3ull << 51;
static const JseRenderState GLS_BACK_STENCIL_OP_FAIL_DECR = 4ull << 51;
static const JseRenderState GLS_BACK_STENCIL_OP_FAIL_INVERT = 5ull << 51;
static const JseRenderState GLS_BACK_STENCIL_OP_FAIL_INCR_WRAP = 6ull << 51;
static const JseRenderState GLS_BACK_STENCIL_OP_FAIL_DECR_WRAP = 7ull << 51;
static const JseRenderState GLS_BACK_STENCIL_OP_FAIL_BITS = 7ull << 51;

static const JseRenderState GLS_BACK_STENCIL_OP_ZFAIL_KEEP = 0ull << 54;
static const JseRenderState GLS_BACK_STENCIL_OP_ZFAIL_ZERO = 1ull << 54;
static const JseRenderState GLS_BACK_STENCIL_OP_ZFAIL_REPLACE = 2ull << 54;
static const JseRenderState GLS_BACK_STENCIL_OP_ZFAIL_INCR = 3ull << 54;
static const JseRenderState GLS_BACK_STENCIL_OP_ZFAIL_DECR = 4ull << 54;
static const JseRenderState GLS_BACK_STENCIL_OP_ZFAIL_INVERT = 5ull << 54;
static const JseRenderState GLS_BACK_STENCIL_OP_ZFAIL_INCR_WRAP = 6ull << 54;
static const JseRenderState GLS_BACK_STENCIL_OP_ZFAIL_DECR_WRAP = 7ull << 54;
static const JseRenderState GLS_BACK_STENCIL_OP_ZFAIL_BITS = 7ull << 54;

static const JseRenderState GLS_BACK_STENCIL_OP_PASS_KEEP = 0ull << 57;
static const JseRenderState GLS_BACK_STENCIL_OP_PASS_ZERO = 1ull << 57;
static const JseRenderState GLS_BACK_STENCIL_OP_PASS_REPLACE = 2ull << 57;
static const JseRenderState GLS_BACK_STENCIL_OP_PASS_INCR = 3ull << 57;
static const JseRenderState GLS_BACK_STENCIL_OP_PASS_DECR = 4ull << 57;
static const JseRenderState GLS_BACK_STENCIL_OP_PASS_INVERT = 5ull << 57;
static const JseRenderState GLS_BACK_STENCIL_OP_PASS_INCR_WRAP = 6ull << 57;
static const JseRenderState GLS_BACK_STENCIL_OP_PASS_DECR_WRAP = 7ull << 57;
static const JseRenderState GLS_BACK_STENCIL_OP_PASS_BITS = 7ull << 57;

static const JseRenderState GLS_SEPARATE_STENCIL = GLS_BACK_STENCIL_OP_FAIL_BITS | GLS_BACK_STENCIL_OP_ZFAIL_BITS | GLS_BACK_STENCIL_OP_PASS_BITS;
static const JseRenderState GLS_STENCIL_OP_BITS = GLS_STENCIL_OP_FAIL_BITS | GLS_STENCIL_OP_ZFAIL_BITS | GLS_STENCIL_OP_PASS_BITS | GLS_SEPARATE_STENCIL;
static const JseRenderState GLS_STENCIL_FRONT_OPS = GLS_STENCIL_OP_FAIL_BITS | GLS_STENCIL_OP_ZFAIL_BITS | GLS_STENCIL_OP_PASS_BITS;
static const JseRenderState GLS_STENCIL_BACK_OPS = GLS_SEPARATE_STENCIL;

static const JseRenderState GLS_DEPTH_TEST_MASK = 1ull << 60;
static const JseRenderState GLS_CLOCKWISE = 1ull << 61;
static const JseRenderState GLS_MIRROR_VIEW = 1ull << 62;
static const JseRenderState GLS_OVERRIDE = 1ull << 63;		// override the render prog state

static const JseRenderState GLS_KEEP = GLS_DEPTH_TEST_MASK;
static const JseRenderState GLS_DEFAULT = 0;

#define STENCIL_SHADOW_TEST_VALUE		128
#define STENCIL_SHADOW_MASK_VALUE		255


class JseGfxCore {
public:
	JseResult Init(bool debugMode);
	JseResult CreateSurface(const JseSurfaceCreateInfo& createSurfaceInfo);
    JseResult CreateBuffer(const JseBufferCreateInfo& createBufferInfo);
    JseResult UpdateBuffer(const JseBufferUpdateInfo& bufferUpdateInfo);
    JseResult DestroyBuffer(JseBufferID bufferId);
    JseResult CreateImage(const JseImageCreateInfo& createImageInfo);
    JseResult CreateTexture(const JseImageCreateInfo& createImageInfo); // Alias for CreateImage
    JseResult UpdateImageData(const JseImageUploadInfo& imgageUploadInfo);
    JseResult CreateGraphicsPipeline(const JseGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo);
    JseResult GetDeviceCapabilities(JseDeviceCapabilities& dest);
    JseResult SetVSyncInterval(int interval);
	void Shutdown();

	virtual ~JseGfxCore() {}
private:
	// data & virtual implementations
	virtual JseResult Init_impl(bool debugMode) = 0;
	virtual JseResult CreateSurface_impl(const JseSurfaceCreateInfo& createSurfaceInfo) = 0;
    virtual JseResult CreateBuffer_impl(const JseBufferCreateInfo& createBufferInfo) = 0;
    virtual JseResult UpdateBuffer_impl(const JseBufferUpdateInfo& bufferUpdateInfo) = 0;
    virtual JseResult DestroyBuffer_impl(JseBufferID bufferId) = 0;
    virtual JseResult CreateImage_impl(const JseImageCreateInfo& createImageInfo) = 0;
    virtual JseResult CreateGraphicsPipeline_impl(const JseGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo) = 0;
    virtual JseResult GetDeviceCapabilities_impl(JseDeviceCapabilities& dest) = 0;
    virtual JseResult UpdateImageData_impl(const JseImageUploadInfo& imgageUploadInfo) = 0;
    virtual JseResult SetVSyncInterval_impl(int interval) = 0;

	virtual void Shutdown_impl() = 0;
};

#endif