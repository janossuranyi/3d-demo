#include "JSE.h"

namespace js {
    Memory::Memory() : data_(nullptr), size_(0) {
    }

    Memory::~Memory()
    {
#ifdef _DEBUG_MEM
        if (size_ > 0) {
            Info("[gfx::Memory] free %d bytes", size_);
        }
#endif
    }

    Memory::Memory(size_t asize) : size_(asize) {
        if (asize > 0) {
            //data_.reset(new uint8_t[asize], std::default_delete<uint8_t[]>());
            data_.reset(reinterpret_cast<uint8_t*>(JseMemAlloc16(asize)), JseMemFree16);
        }
    }

    size_t Memory::size() const {
        return size_;
    }

    bool Memory::empty() const
    {
        return size() == 0;
    }

    uint8_t* Memory::data() const {
        return data_.get();
    }

    uint8_t Memory::operator[](size_t index) const
    {
        return data_.get()[index];
    }
}
