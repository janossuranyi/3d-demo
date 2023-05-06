#ifndef JSR_HEAP_H
#define JSR_HEAP_H

namespace jsr {

	void* MemAlloc16(const size_t size);
	void MemFree16(void* ptr);
	void* MemClearedAlloc(const size_t size);

	inline void* MemAlloc(const size_t size) { return MemAlloc16(size); }
	inline void MemFree(void* ptr) { MemFree16(ptr); }
}

#endif