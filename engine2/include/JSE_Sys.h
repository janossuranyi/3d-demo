#ifndef JSE_SYS_H
#define JSE_SYS_H

#include <SDL.h>
#include <cinttypes>

enum JseMessageBoxFlags : uint32_t {
    JseMESSAGEBOX_ERROR,
    JseMESSAGEBOX_WARNING,
    JseMESSAGEBOX_INFO
};

extern SDL_Window* p_defaultWindow;

/*
Here are the names returned for some (but not all) supported platforms:

    "Windows"
    "Mac OS X"
    "Linux"
    "iOS"
    "Android"
*/
const char* JseGetPlatform();

/*
Determine the L1 cache line size of the CPU.
*/
int JseGetCPUCacheLineSize();

int JseGetCpuCount();
int JseGetSystemRAM();
int JseMessageBox(uint32_t flags, const char* title, const char* message);

#define assert_2_byte_aligned( ptr )		assert( ( ((uintptr_t)(ptr)) &  1 ) == 0 )
#define assert_4_byte_aligned( ptr )		assert( ( ((uintptr_t)(ptr)) &  3 ) == 0 )
#define assert_8_byte_aligned( ptr )		assert( ( ((uintptr_t)(ptr)) &  7 ) == 0 )
#define assert_16_byte_aligned( ptr )		assert( ( ((uintptr_t)(ptr)) & 15 ) == 0 )
#define assert_32_byte_aligned( ptr )		assert( ( ((uintptr_t)(ptr)) & 31 ) == 0 )
#define assert_64_byte_aligned( ptr )		assert( ( ((uintptr_t)(ptr)) & 63 ) == 0 )
#define assert_128_byte_aligned( ptr )		assert( ( ((uintptr_t)(ptr)) & 127 ) == 0 )
#define assert_aligned_to_type_size( ptr )	assert( ( ((uintptr_t)(ptr)) & ( sizeof( (ptr)[0] ) - 1 ) ) == 0 )

/*
================================================================================================

	Zero cache line and prefetch intrinsics

================================================================================================
*/

#if defined(USE_INTRINSICS_SSE)
// The code below assumes that a cache line is 64 bytes.
#define CACHE_LINE_SIZE						64

inline void Prefetch(const void* ptr, int offset)
{
	//	const char * bytePtr = ( (const char *) ptr ) + offset;
	//	_mm_prefetch( bytePtr +  0, _MM_HINT_NTA );
	//	_mm_prefetch( bytePtr + 64, _MM_HINT_NTA );
}
inline void ZeroCacheLine(void* ptr, int offset)
{
	assert_64_byte_aligned(ptr);
	char* bytePtr = ((char*)ptr) + offset;
	__m128i zero = _mm_setzero_si128();
	_mm_store_si128((__m128i*)(bytePtr + 0 * 16), zero);
	_mm_store_si128((__m128i*)(bytePtr + 1 * 16), zero);
	_mm_store_si128((__m128i*)(bytePtr + 2 * 16), zero);
	_mm_store_si128((__m128i*)(bytePtr + 3 * 16), zero);
#if CACHE_LINE_SIZE == 128
	_mm_store_si128((__m128i*)(bytePtr + 4 * 16), zero);
	_mm_store_si128((__m128i*)(bytePtr + 5 * 16), zero);
	_mm_store_si128((__m128i*)(bytePtr + 6 * 16), zero);
	_mm_store_si128((__m128i*)(bytePtr + 7 * 16), zero);
#endif
}
inline void FlushCacheLine(const void* ptr, int offset)
{
	const char* bytePtr = ((const char*)ptr) + offset;
	_mm_clflush(bytePtr + 0);
	_mm_clflush(bytePtr + 64);
}

/*
================================================
#endif
	Other
================================================
*/
#else

#define CACHE_LINE_SIZE						64

inline void Prefetch(const void* ptr, int offset) {}
inline void ZeroCacheLine(void* ptr, int offset)
{
	char* bytePtr = (char*)((((uintptr_t)(ptr)) + (offset)) & ~(CACHE_LINE_SIZE - 1));
	memset(bytePtr, 0, CACHE_LINE_SIZE);
}
inline void FlushCacheLine(const void* ptr, int offset) {}

#endif

#endif