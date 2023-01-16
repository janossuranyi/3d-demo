#ifndef JSE_GFX_CORE_H
#define JSE_GFX_CORE_H

struct JseBufferTag{};
struct JseImageTag{};
struct JseVertexInputTag{};
struct JseGraphicsPipelineTag{};

using JseBufferID = JseHandle<JseBufferTag, -1>;
using JseImageID = JseHandle<JseImageTag, -1>;
using JseVertexInputID = JseHandle<JseVertexInputTag, -1>;
using JseGrapicsPipelineID = JseHandle<JseGraphicsPipelineTag, -1>;

enum JseBufferStorageFlags {
    JSE_BUFFER_STORAGE_PERSISTENT_BIT = 1,
    JSE_BUFFER_STORAGE_COHERENT_BIT = 2,
    JSE_BUFFER_STORAGE_READ_BIT = 4,
    JSE_BUFFER_STORAGE_WRITE_BIT = 8,
    JSE_BUFFER_STORAGE_DYNAMIC_BIT = 16
};

enum class JseFilter { Nearest, Linear, NearestLinear, LinearNearest, LinearLinear };

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

    float maxAnisotropy;
};

struct JseImageCreateInfo {
    JseImageID imageId;
    JseImageTarget target;
    JseFormat format;
    JseSamplerDescription samplerDescription;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t mipCount;
    uint32_t arrayCount;
};

struct JseImageUploadInfo {
    JseImageID imageId;
    uint32_t depth;
    uint32_t mipLevel;
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

struct JseGraphicsPipelineCreateInfo {
    JseGrapicsPipelineID graphicsPipelineId;
    uint32_t bindingCount;
    JseVertexInputBindingDescription* bindings;
    uint32_t attributeCount;
    JseVertexInputAttributeDescription* attributes;    
};

class JseGfxCore {
public:
	JseResult Init(bool debugMode);
	JseResult CreateSurface(const JseSurfaceCreateInfo& createSurfaceInfo);
    JseResult CreateBuffer(const JseBufferCreateInfo& createBufferInfo);
    JseResult UpdateBuffer(const JseBufferUpdateInfo& bufferUpdateInfo);
    JseResult DestroyBuffer(JseBufferID bufferId);
    JseResult CreateImage(const JseImageCreateInfo& createImageInfo);
    JseResult CreateGraphicsPipeline(const JseGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo);

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
	virtual void Shutdown_impl() = 0;
};

#endif