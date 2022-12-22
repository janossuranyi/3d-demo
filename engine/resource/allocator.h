#pragma once

void* Mem_Alloc16(const size_t size);
void Mem_Free16(void* ptr);
void* Mem_ClearedAlloc(const size_t size);

inline void* Mem_Alloc(const size_t size) { return Mem_Alloc16(size); }
inline void Mem_Free(void* ptr) { Mem_Free16(ptr); }

