#pragma once

#include <cinttypes>
#include <memory>
namespace jsr {

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
    enum class Format {
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

	enum class Result
	{
		success, failed
	};

    enum class eCullMode { none, front, back, front_back };
    enum class eCullFrontFace { cw, ccw };
    enum class ePolygonMode { fill, wireframe };
    enum class eVertexInputRate { vertex, instance };
    enum class eShaderStage { vertex, geometry, fragment, compute };
    enum class eCompareOp { less, lessThan, greater, greaterThan, equal, always, never };
    enum class eBlendOp { add, subtract, reverse_subtract, min, max };
    enum class eBlendFactor {
        zero,one,
        src_color,
        one_minus_src_color,
        dst_color,
        one_minus_dst_color,
        src_alpha,
        one_minus_src_alpha,
        dst_alpha,
        one_minus_dst_alpha,
        constant_color,
        one_minus_constant_color,
        constant_alpha,
        one_minus_constant_alpha,
        src_alpha_saturate,
        src1_color,
        one_minus_src1_color,
        src1_alpha,
        one_minus_src_alpha
    };

	typedef std::shared_ptr<void> GpuBufferHandle;
    typedef std::shared_ptr<void> GpuShaderHandle;
    typedef std::shared_ptr<const void> SharedPtr;
    typedef void* RenderPassHandle;
    typedef void* GraphicsPipelineHandle;

	struct GpuCapabilities {
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

    struct VertexInputBinding
    {
        int binding;
        int stride;
        eVertexInputRate inputRate;
    };

    struct VertexInputAttribute
    {
        int location;
        int binding;
        int offset;
        Format format;
    };

    struct VertexInputDef
    {
        int                 bindingCount;
        const VertexInputBinding* pBindings;
        int                 attribCount;
        const VertexInputAttribute* pAttributes;
    };

    struct ShaderStageDef
    {
        eShaderStage    stage;
        int             size;
        void*           code;
    };
    
    struct RenderPassDef {};
    struct DepthStencilStateDef
    {
        bool depthTestEnable;
        bool depthWriteEnable;
        eCompareOp depthCompareOp;
    };
    struct BlendStateDef
    {
        bool blendEnable;
        eBlendOp colorBlendOp;
        eBlendOp alphaBlendOp;
        eBlendFactor srcBlendFactor;
        eBlendFactor dstBlendFactor;
        int colorWriteMask;
    };
    struct RasterizationStateDef
    {
        bool depthClampEnable;
        bool rasterizerDiscardEnable;
        ePolygonMode polygonMode;
        eCullMode    cullMode;
        eCullFrontFace frontFace;
        float lineWidth;
        float pointSize;
    };
    struct GraphicsPipelineDef
    {
        int                             stageCount;
        const GpuShaderHandle*          pStages;
        const VertexInputDef*           pVertexInputState;
        RenderPassHandle                renderPass;
        const DepthStencilStateDef*     pDepthStencilState;
        const BlendStateDef*            pBlendState;
        const RasterizationStateDef*    pRasterizationState;
    };

    struct RenderPassDef {};

	class GfxCore
	{
	public:
		GfxCore();
		~GfxCore();
		Result		CreateWindow(int width, int height, int fullscreen = 0);
		Result		CreateVertexBuffer(int size, GpuBufferHandle& handle);
		Result		CreateIndexBuffer(int size, GpuBufferHandle& handle);
        Result		CreateUniformBuffer(int size, GpuBufferHandle& handle);
        Result      CreateShader(const ShaderStageDef& def, GpuShaderHandle& handle);
        Result      CreateGraphicsPipeline(const GraphicsPipelineDef& def, GraphicsPipelineHandle& handle);

        Result		CmdUpdateBuffer(GpuBufferHandle handle, SharedPtr data, int offset, int size);
		void		GetScreenSize(int& x, int& y) const;
		bool		IsInitialized() const;
		void		gc() const;
	private:
		void* pInternals;
	};
}