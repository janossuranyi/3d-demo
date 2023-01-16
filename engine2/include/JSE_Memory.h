#ifndef JSE_MEMORY_H
#define JSE_MEMORY_H

#include <cassert>
#include <vector>
#include <memory>
#include <cinttypes>

class JseMemory {
public:
    /// Constructs an empty block
    JseMemory();

    /// Destroys memory block.
    ~JseMemory();

    // Copyable.
    JseMemory(const JseMemory&) = default;
    JseMemory& operator=(const JseMemory&) = default;

    // Movable.
    JseMemory(JseMemory&&) = default;
    JseMemory& operator=(JseMemory&&) = default;

    /// Constructs a block with asize bytes allocated
    explicit JseMemory(size_t asize);

    /// Creates a memory block by copying from existing data.
    /// Size in bytes.
    template <typename T> JseMemory(const T* data, size_t size);

    /// Creates a memory block by copying from existing data.
    template <typename T> explicit JseMemory(const std::vector<T>& data);

    /// Returns memory buffer size
    /// <returns>bytes allocated</returns>
    size_t size() const;

    /// Returns true if memory not contains any data
    /// <returns>true/false</returns>
    bool empty() const;

    /// Returns a pointer to the buffer data
    /// <returns>pointer</returns>
    uint8_t* data() const;

    /// Get a byte from memory indexed by "index"
    /// <param name="index">Memory offset</param>
    /// <returns>a byte</returns>
    uint8_t operator[](size_t index) const;

    /// Get a T from memory indexed by "index"
    /// <param name="index">Memory offset</param>
    /// <returns>a T</returns>
    template<typename T>
    T operator[](size_t index) const;

    /// Get a T from memory indexed by "index"
    /// <param name="index">Memory offset</param>
    /// <returns>a const T&</returns>
    template<typename T>
    const T& operator[](size_t index) const;

    /// Get a T from memory indexed by "index"
    /// <param name="index">Memory offset</param>
    /// <returns>a T&</returns>
    template<typename T>
    T& operator[](size_t index) const;

private:
    std::shared_ptr<uint8_t> data_;
    size_t size_;

};

template<typename T>
T JseMemory::operator[](size_t index) const {
    assert(index * sizeof(T) < size_);
    return reinterpret_cast<T*>(data_.get())[index];
}

template<typename T>
const T& JseMemory::operator[](size_t index) const {
    assert(index * sizeof(T) < size_);
    return reinterpret_cast<T*>(data_.get())[index];
}

template<typename T>
T& JseMemory::operator[](size_t index) const {
    assert(index * sizeof(T) < size_);
    return reinterpret_cast<T*>(data_.get())[index];
}

template<typename T>
JseMemory::JseMemory(const T* data, size_t size) : JseMemory(data ? size : 0) {
    if (data != nullptr) {
        std::memcpy(data_.get(), data, size);
    }
}

template<typename T>
JseMemory::JseMemory(const std::vector<T>& data) : JseMemory(data.data(), data.size() * sizeof(T)) {
};


#endif