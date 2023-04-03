#ifndef JSE_HEAP_H
#define JSE_HEAP_H

void* JseMemAlloc16(const size_t size);
void JseMemFree16(void* ptr);
void* JseMemClearedAlloc(const size_t size);

inline void* JseMemAlloc(const size_t size) { return JseMemAlloc16(size); }
inline void JseMemFree(void* ptr) { JseMemFree16(ptr); }

#endif