#ifndef JSE_GFX_CORE_H
#define JSE_GFX_CORE_H

#include <variant>

struct JseBufferTag{};
struct JseFrameBufferTag {};
struct JseSamplerTag {};
struct JseImageTag{};
struct JseShaderTag{};
struct JseVertexInputTag{};
struct JseGraphicsPipelineTag{};
struct JseDescriptorSetLayoutTag{};
struct JseDescriptorSetTag {};
struct JseFenceTag {};

using JseFenceID = JseHandle<JseFenceTag, -1>;
using JseDescriptorSetLayoutID = JseHandle<JseDescriptorSetLayoutTag, -1>;
using JseDescriptorSetID = JseHandle<JseDescriptorSetTag, -1>;
using JseBufferID = JseHandle<JseBufferTag, -1>;
using JseFrameBufferID = JseHandle<JseFrameBufferTag, -1>;
using JseImageID = JseHandle<JseImageTag, -1>;
using JseSamplerID = JseHandle<JseSamplerTag, -1>;
using JseVertexInputID = JseHandle<JseVertexInputTag, -1>;
using JseGrapicsPipelineID = JseHandle<JseGraphicsPipelineTag, -1>;
using JseShaderID = JseHandle<JseShaderTag, -1>;
using JseRenderState = uint64_t;
using JseDeviceSize = uint64_t;

using JseUniformData = std::variant<int, float, glm::ivec2, glm::ivec3, glm::ivec4, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4, JseVector<float>, JseVector<glm::vec4>>;
using JseUniformMap = JseHashMap<JseString, JseUniformData>;

struct JseColor4f {
    float r, g, b, a;
    bool operator==(const JseColor4f& x) {
        if (r != x.r) return false;
        if (g != x.g) return false;
        if (b != x.b) return false;
        if (a != x.a) return false;
        return true;
    }
    bool operator!=(const JseColor4f& x) {
        return !operator==(x);
    }
};

struct JseRect2D {
    int x;
    int y;
    int w;
    int h;
};

typedef union JseClearValue {
    glm::vec4 color;
    float depth;
    int stencil;
} JseClearValue;

enum class JseAccess { READ, WRITE, READ_WRITE };

// Primitives
enum class JseTopology { Point, Lines, LineStrip, LineLoop, Triangles, TriangleFan, TriangleStrip };

enum class JseIndexType {
    UINT16, UINT32
};

enum class JseCubeMapFace {
    POSITIVE_X,
    NEGATIVE_X,
    POSITIVE_Y,
    NEGATIVE_Y,
    POSITIVE_Z,
    NEGATIVE_Z
};

const JseCubeMapFace JSE_CUBE_MAP_FACES[] {
    JseCubeMapFace::POSITIVE_X,
    JseCubeMapFace::NEGATIVE_X,
    JseCubeMapFace::POSITIVE_Y,
    JseCubeMapFace::NEGATIVE_Y,
    JseCubeMapFace::POSITIVE_Z,
    JseCubeMapFace::NEGATIVE_Z
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

enum JseShaderStageFlags {
    JSE_STAGE_FLAG_VERTEX = 1,
    JSE_STAGE_FLAG_FRAGMENT = 2,
    JSE_STAGE_FLAG_GEOMETRY = 4,
    JSE_STAGE_FLAG_TESSELATION_CONTROL = 8,
    JSE_STAGE_FLAG_TESSELATION = 16,
    JSE_STAGE_FLAG_COMPUTE = 32,
    JSE_STAGE_FLAG_ALL = 0xFFFFFFFF
};

/*
Resorce types:

1. Vertex buffer
2. Index buffer
3. Uniform buffer
4. Shader Storage buffer
5. Image Load/Store
6. Inline uniform block

*/

enum class JseDescriptorType {
    UNIFORM_BUFFER,
    UNIFORM_BUFFER_DYNAMIC,
    STORAGE_BUFFER,
    STORAGE_BUFFER_DYNAMIC,
    STORAGE_IMAGE,
    SAMPLED_IMAGE,
    SAMPLED_BUFFER,
    INLINE_UNIFORM_BLOCK
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
    RGB32F,
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
    RGB_DXT1,
    RGBA_DXT1,
    RGBA_DXT3,
    RGBA_DXT5,
    RGBA_BPTC,
    RGB_ASTC_6x5,
    RGB_ASTC_6x6,
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
    bool srgb;
};

struct JseBufferCreateInfo {
    JseBufferID bufferId;
    JseBufferTarget target;
    uint32_t storageFlags;
    JseFormat format;
    uint32_t size;
};

struct JseBufferUpdateInfo {
    JseBufferID bufferId;
    uint32_t offset;
    uint32_t size;
    uint8_t* data;
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
    JseBufferID buffer;
    uint32_t width;
    uint32_t height;    // 2D height or Array size
    uint32_t depth;     // 3D depth or Array size
    uint32_t levelCount;
    JseDeviceSize offset;   // Buffer Texture offset
    JseDeviceSize size;     // Buffer Texture size
    bool srgb;
    bool immutable;
    bool compressed;
};

struct JseImageUploadInfo {
    JseImageID imageId;
    uint32_t level;
    uint32_t face;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t xoffset;
    uint32_t yoffset;
    uint32_t zoffset;
    uint32_t imageSize;
    uint8_t* data;
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

struct JseShaderCreateInfo {
    JseShaderID shaderId;
    JseShaderStage stage;
    uint32_t codeSize;
    const void* pCode;    
};

struct JsePipelineShaderStageCreateInfo {
    JseShaderStage stage;
    JseShaderID shader;
};

struct JsePipelineVertexInputStateCreateInfo {
    uint32_t bindingCount;
    JseVertexInputBindingDescription* pBindings;
    uint32_t attributeCount;
    JseVertexInputAttributeDescription* pAttributes;
};

struct JseGraphicsPipelineCreateInfo {
    JseGrapicsPipelineID graphicsPipelineId;
    JsePipelineVertexInputStateCreateInfo* pVertexInputState;
    uint32_t stageCount;
    JsePipelineShaderStageCreateInfo* pStages;
    JseRenderState renderState;
    JseDescriptorSetLayoutID setLayoutId;
};

struct JseFrameBufferAttachmentDescription {
    JseImageID image;
    JseCubeMapFace face;
    uint32_t level;
    uint32_t layer;
};

struct JseFrameBufferCreateInfo {
    JseFrameBufferID frameBufferId;
    uint32_t width;
    uint32_t height;
    uint32_t colorAttachmentCount;
    JseFrameBufferAttachmentDescription* pColorAttachments;
    JseFrameBufferAttachmentDescription* pDepthAttachment;
    JseFrameBufferAttachmentDescription* pStencilAttachment;
};

struct JseRenderPassInfo {
    JseFrameBufferID framebuffer;
    JseRect2D viewport;
    JseRect2D scissor;
    bool colorClearEnable;
    bool depthClearEnable;
    bool stencilClearEnable;
    bool scissorEnable;
    JseClearValue colorClearValue;
    JseClearValue depthClearValue;
    JseClearValue stencilClearValue;
};

struct JseDescriptorSetLayoutBinding {
    uint32_t binding;
    JseDescriptorType descriptorType;
    uint32_t descriptorCount;
    JseShaderStageFlags stageFlags;
};

struct JseDescriptorSetLayoutCreateInfo {
    JseDescriptorSetLayoutID setLayoutId;
    uint32_t bindingCount;
    JseDescriptorSetLayoutBinding* pBindings;
};

struct JseDescriptorImageInfo {
    JseImageID image;
    int level;
    bool layered;
    int layer;
    JseAccess access;
    JseFormat format;
};

struct JseDescriptorBufferInfo {
    JseBufferID buffer;
    JseDeviceSize offset;
    JseDeviceSize size;
};

struct JseDescriptorUniformInfo {
    uint32_t vectorCount;
    glm::vec4* pVectors;
    JseUniformData value;
    char name[32];
};

static_assert(sizeof(JseDescriptorUniformInfo) <= 2 * std::hardware_destructive_interference_size, "");

struct JseWriteDescriptorSet {
    JseDescriptorSetID setId;
    uint32_t dstBinding;
    uint32_t dstArrayElement;
    uint32_t descriptorCount;
    JseDescriptorType descriptorType;
    JseDescriptorImageInfo* pImageInfo;
    JseDescriptorBufferInfo* pBufferInfo;
    JseDescriptorUniformInfo* pUniformInfo;
};

struct JseDescriptorSetCreateInfo {
    JseDescriptorSetID setId;
    JseDescriptorSetLayoutID setLayoutId;
};

struct JseDeviceCapabilities {
    const char* pRenderer;
    const char* pRendererVersion;
    int maxFragmentTextureImageUnits;
    int maxVertexTextureImageUnits;
    int maxComputeTextureImageUnits;
    int maxArrayTextureLayers;
    int maxTextureSize;
    int maxComputeSharedMemorySize;
    int maxUniformBlockSize;
    int maxShaderStorageBlockSize;
    int maxVertexAttribs;
    int maxVertexAttribBindings;
    int uniformBufferOffsetAligment;
    int availableVideoMemory;
};


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
    JseResult DeleteImage(JseImageID imageId);
    JseResult CreateGraphicsPipeline(const JseGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo);
    JseResult DeleteGraphicsPipeline(JseGrapicsPipelineID pipelineId);
    JseResult BindGraphicsPipeline(JseGrapicsPipelineID pipelineId);
    JseResult CreateFrameBuffer(const JseFrameBufferCreateInfo& frameBufferCreateInfo);
    JseResult DeleteFrameBuffer(JseFrameBufferID framebufferId);
    JseResult CreateShader(const JseShaderCreateInfo& shaderCreateInfo, std::string& errorOutput);
    JseResult BeginRenderPass(const JseRenderPassInfo& renderPassInfo);
    JseResult CreateDescriptorSetLayout(const JseDescriptorSetLayoutCreateInfo& cmd);
    JseResult EndRenderPass();
    JseResult CreateDescriptorSet(const JseDescriptorSetCreateInfo& cmd);
    JseResult WriteDescriptorSet(const JseWriteDescriptorSet& cmd);
    JseResult BindDescriptorSet(uint32_t firstSet, uint32_t descriptorSetCount, const JseDescriptorSetID* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets);
    JseResult CreateFence(JseFenceID id);
    JseResult DeleteFence(JseFenceID id);
    JseResult WaitSync(JseFenceID id, uint64_t timeout);

    void SwapChainNextImage();
    void BeginRendering();
    void EndRendering();

    void BindVertexBuffer(uint32_t binding, JseBufferID buffer, JseDeviceSize offsets);
    void BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const JseBufferID* pBuffers, const JseDeviceSize* pOffsets);
    void BindIndexBuffer(JseBufferID buffer, uint32_t offset, JseIndexType type);
    void Draw(JseTopology mode, uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
    void DrawIndexed(JseTopology mode, uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);
    void Viewport(const JseRect2D& x);
    void Scissor(const JseRect2D& x);
    JseResult GetDeviceCapabilities(JseDeviceCapabilities& dest);
    JseResult SetVSyncInterval(int interval);
    JseResult GetSurfaceDimension(glm::ivec2& x);

    void* GetMappedBufferPointer(JseBufferID id);

	void Shutdown();

	virtual ~JseGfxCore() {}
private:
	// data & virtual implementations
    virtual void* GetMappedBufferPointer_impl(JseBufferID id) = 0;
	virtual JseResult Init_impl(bool debugMode) = 0;
	virtual JseResult CreateSurface_impl(const JseSurfaceCreateInfo& createSurfaceInfo) = 0;
    virtual JseResult CreateBuffer_impl(const JseBufferCreateInfo& createBufferInfo) = 0;
    virtual JseResult UpdateBuffer_impl(const JseBufferUpdateInfo& bufferUpdateInfo) = 0;
    virtual JseResult DestroyBuffer_impl(JseBufferID bufferId) = 0;
    virtual JseResult CreateImage_impl(const JseImageCreateInfo& createImageInfo) = 0;
    virtual JseResult DeleteImage_impl(JseImageID imageId) = 0;
    virtual JseResult CreateGraphicsPipeline_impl(const JseGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo) = 0;
    virtual JseResult BindGraphicsPipeline_impl(JseGrapicsPipelineID pipelineId) = 0;
    virtual JseResult DeleteGraphicsPipeline_impl(JseGrapicsPipelineID pipelineId) = 0;
    virtual JseResult GetDeviceCapabilities_impl(JseDeviceCapabilities& dest) = 0;
    virtual JseResult UpdateImageData_impl(const JseImageUploadInfo& imgageUploadInfo) = 0;
    virtual JseResult CreateShader_impl(const JseShaderCreateInfo& shaderCreateInfo, std::string& errorOutput) = 0;
    virtual JseResult CreateFrameBuffer_impl(const JseFrameBufferCreateInfo& frameBufferCreateInfo) = 0;
    virtual JseResult DeleteFrameBuffer_impl(JseFrameBufferID framebufferId) = 0;
    virtual JseResult BeginRenderPass_impl(const JseRenderPassInfo& renderPassInfo) = 0;
    virtual JseResult CreateDescriptorSetLayout_impl(const JseDescriptorSetLayoutCreateInfo& cmd) = 0;
    virtual JseResult BindDescriptorSet_impl(uint32_t firstSet, uint32_t descriptorSetCount, const JseDescriptorSetID* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) = 0;
    virtual JseResult EndRenderPass_impl() = 0;
    virtual JseResult CreateDescriptorSet_impl(const JseDescriptorSetCreateInfo& cmd) = 0;
    virtual JseResult WriteDescriptorSet_impl(const JseWriteDescriptorSet& cmd) = 0;
    virtual JseResult CreateFence_impl(JseFenceID id) = 0;
    virtual JseResult DeleteFence_impl(JseFenceID id) = 0;
    virtual JseResult WaitSync_impl(JseFenceID id, uint64_t time) = 0;

    virtual void SwapChainNextImage_impl() = 0;
    virtual void BeginRendering_impl() = 0;
    virtual void EndRendering_impl() = 0;

    virtual void BindVertexBuffers_impl(uint32_t firstBinding, uint32_t bindingCount, const JseBufferID* pBuffers, const JseDeviceSize* pOffsets) = 0;
    virtual void BindVertexBuffer_impl(uint32_t binding, JseBufferID buffer, JseDeviceSize offsets) = 0;
    virtual void BindIndexBuffer_impl(JseBufferID buffer, uint32_t offset, JseIndexType type) = 0;
    virtual void Draw_impl(JseTopology mode, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
    virtual void DrawIndexed_impl(JseTopology mode, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) = 0;
    virtual void Viewport_impl(const JseRect2D& x) = 0;
    virtual void Scissor_impl(const JseRect2D& x) = 0;
    virtual JseResult SetVSyncInterval_impl(int interval) = 0;
    virtual JseResult GetSurfaceDimension_impl(glm::ivec2&) = 0;

	virtual void Shutdown_impl() = 0;
};

#endif