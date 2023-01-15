#ifndef JSE_HEAP_H
#define JSE_HEAP_H

void* JSE_MemAlloc16(const size_t size);
void JSE_MemFree16(void* ptr);
void* JSE_MemClearedAlloc(const size_t size);

inline void* JSE_MemAlloc(const size_t size) { return JSE_MemAlloc16(size); }
inline void JSE_MemFree(void* ptr) { JSE_MemFree16(ptr); }

#endif