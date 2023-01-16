#include "JSE.h"

void* JseMemAlloc16(const size_t size)
{
    if (!size)
    {
        return NULL;
    }

    const size_t padded = (size + 15) & ~15;
#ifdef _WIN32
    return _aligned_malloc(padded, 16);
#else
    void* ret;
    posix_memalign(&ret, 16, padded);
    return ret;
#endif
}

void JseMemFree16(void* p)
{
    if (p == NULL)
    {
        return;
    }
#ifdef _WIN32
    _aligned_free(p);
#else
    free(p);
#endif
}

void* JseMemClearedAlloc(const size_t size)
{
    void* mem = JseMemAlloc16(size);
    memset(mem, 0, size);

    return mem;
}
