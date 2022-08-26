#pragma once

#define INVALID_BUFFER 0
#define INVALID_TEXTURE 0

#define UNIFORM_BUFFER_ALIGN 127
#define INDEX_BUFFER_ALIGN 15
#define VERTEX_BUFFER_ALIGN 31

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
// eGpuBufferAccess 

const unsigned int BA_DYNAMIC = 1;
const unsigned int BA_MAP_READ = 2;
const unsigned int BA_MAP_WRITE = 4; 
const unsigned int BA_MAP_PERSISTENT = 8;
const unsigned int BA_MAP_COHERENT = 16;
const unsigned int BA_WRITE_PERSISTENT = (BA_DYNAMIC | BA_MAP_WRITE | BA_MAP_PERSISTENT);
const unsigned int BA_WRITE_PERSISTENT_COHERENT = (BA_DYNAMIC | BA_MAP_WRITE | BA_MAP_PERSISTENT | BA_MAP_COHERENT);


/*
Drawing related types
*/

enum class eDrawMode { POINTS, LINE_STRIP, LINE_LOOP, LINES, TRIANGLE_STRIP, TRIANGLE_FAN, TRIANGLES };