#include <memory>
#include "./Heap.h"

namespace jsr {

    void* MemAlloc16(const size_t size)
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

    void MemFree16(void* p)
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

    void* MemClearedAlloc(const size_t size)
    {
        void* mem = MemAlloc16(size);
        memset(mem, 0, size);

        return mem;
    }

}