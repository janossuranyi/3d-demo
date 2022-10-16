#include "memory.h"

Memory::Memory() : data_(nullptr), size_(0) {
}

Memory::Memory(size_t asize) : size_(asize) {
	if (asize > 0) {
		data_.reset(new uint8_t[asize], std::default_delete<uint8_t[]>());
	}
}

size_t Memory::size() const {
	return size_;
}

uint8_t* Memory::data() const {
	return data_.get();
}

uint8_t Memory::operator[](size_t index) const
{
	return data_.get()[index];
}
