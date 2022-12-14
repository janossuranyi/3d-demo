#pragma once

#define INVALID_BUFFER 0xFFFF
#define INVALID_TEXTURE 0xFFFF

/*
GPU related data types
*/
enum class eDataType { BYTE, UNSIGNED_BYTE, SHORT, UNSIGNED_SHORT, INT32, UNSIGNED_INT32, FLOAT, HALF_FLOAT, UNSIGNED_INT_24_8};
enum class ePixelFormat { RGB, RGBA, RGB8, RGBA8, RGBA16 };

/*
Texture related types
*/
enum class eTextureTarget { TEX_1D, TEX_2D, TEX_3D, TEX_CUBE_MAP };
enum class eTextureFormat { R, R16, R16F, RG, RG16, RG16F, RGB, RGBA, SRGB, SRGB_A, RGBA16F, RGB10A2, RGBA32F, DEPTH24_STENCIL_8, COMPRESSED_RGBA, COMPRESSED_SRGB, R11F_G11F_B10F, RGB5_A1, RGB565 };
enum class eTexMinFilter { NEAREST, LINEAR, NEAREST_MIPMAP_NEAREST, LINEAR_MIPMAP_NEAREST, NEAREST_MIPMAP_LINEAR, LINEAR_MIPMAP_LINEAR };
enum class eTexMagFilter { NEAREST, LINEAR };
enum class eTexWrap { CLAMP_TO_BORDER, MIRRORED_REPEAT, REPEAT, MIRROR_CLAMP_TO_EDGE, CLAMP_TO_EDGE };
enum class eImageAccess { READ_ONLY, WRITE_ONLY, READ_WRITE };
enum class eImageFormat { RGBA32F, RGBA16F, RGBA8 };
/*
GPU Shader related types
*/
enum class eShaderStage { VERTEX, FRAGMENT, GEOMETRY, TESS_CONTROL, TESS_EVALUATION, COMPUTE };

/*
GPU Buffer related types
*/

enum class eGpuBufferTarget { VERTEX, INDEX, UNIFORM, ENUM_SIZE };
enum class eGpuBufferUsage { STATIC, DYNAMIC, DEFAULT };
enum eGpuBufferAccess { BA_DYNAMIC = 1, BA_MAP_READ = 2, BA_MAP_WRITE = 4, BA_MAP_PERSISTENT = 8, BA_MAP_COHERENT = 16 };

/*
Drawing related types
*/

enum class eDrawMode { POINTS, LINE_STRIP, LINE_LOOP, LINES, TRIANGLE_STRIP, TRIANGLE_FAN, TRIANGLES };