#ifndef JSE_GFX_CORE_H
#define JSE_GFX_CORE_H

enum JSE_MapBufferFlagBits {
    JSE_MAP_BUFFER_PERSISTENT_BIT = 1,
    JSE_MAP_BUFFER_COHERENT_BIT = 2,
    JSE_MAP_BUFFER_READ_BIT = 4,
    JSE_MAP_BUFFER_WRITE_BIT = 8
};

enum class JSE_Result {
	SUCCESS = 0,
	GENERIC_ERROR = 1,
	OUT_OF_MEMORY_ERROR = 0x10000,
	NOT_IMPLEMENTED = 0x10001,
	INVALID_SURFACE_DIMENSION = 0x20000
};

enum class JSE_BufferTarget {
	VERTEX,
	INDEX,
	UNIFORM,
	UNIFORM_DYNAMIC,
	SHADER_STORAGE,
	SHADER_STORAGE_DYNAMIC,
	TEXTURE,
	FEEDBACK
};

enum class JSE_ImageTarget {
	D1,
	D2,
	D3,
	CUBEMAP,
	BUFFER,
	SHADER
};

enum class JSE_ShaderStage {
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

enum class JSE_ImageFormat {
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

struct JSE_CreateSurfaceInfo {
	int width;
	int height;
	int colorBits;
	int depthBits;
	int alphaBits;
	int stencilBits;
	bool fullScreen;
};

struct JSE_CreateBufferInfo {
    JSE_BufferTarget target;
    JSE_MapBufferFlagBits storageFlags;

};

struct JSE_CreateImageInfo {

};

/****************************************/
/*  Internal data structures            */
/****************************************/
struct _JSE_SurfaceData {
	int width;
	int height;
	int redBits;
	int greenBits;
	int blueBits;
	int alphaBits;
};

class JSE_GfxCore {
public:
	JSE_Result Init();
	JSE_Result CreateSurface(const JSE_CreateSurfaceInfo& createSurfaceInfo);
	void Shutdown();

	virtual ~JSE_GfxCore() {}
private:
	// data & virtual implementations
	virtual JSE_Result Init_impl() = 0;
	virtual JSE_Result CreateSurface_impl(const JSE_CreateSurfaceInfo& createSurfaceInfo) = 0;
	virtual void Shutdown_impl() = 0;
};

#endif