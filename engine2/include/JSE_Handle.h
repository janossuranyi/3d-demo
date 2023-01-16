#ifndef JSE_HANDLE_H
#define JSE_HANDLE_H

#include <cinttypes>
#include <limits>
#include <vector>

template<typename Tag, int Invalid>
class JseHandle {
public:
    using base_type = uint32_t;
    static const base_type invalid = static_cast<base_type>(Invalid);

    JseHandle() : internal_handle_(Invalid) {
    }

    explicit JseHandle(base_type ahandle) : internal_handle_(ahandle) {
    }

    explicit operator base_type() const {
        return internal();
    }

    base_type internal() const {
        return internal_handle_;
    }

    JseHandle<Tag, Invalid>& operator=(base_type other) {
        internal_handle_ = other;
        return *this;
    }

    bool operator==(JseHandle<Tag, Invalid>& other) const {
        return internal_handle_ == other.internal_handle_;
    }

    bool operator==(const JseHandle<Tag, Invalid>& other) const {
        return internal_handle_ == other.internal_handle_;
    }

    bool operator==(base_type other) const {
        return internal_handle_ == other;
    }

    bool operator!=(JseHandle<Tag, Invalid>& other) const {
        return internal_handle_ != other.internal_handle_;
    }

    bool operator!=(const JseHandle<Tag, Invalid>& other) const {
        return internal_handle_ != other.internal_handle_;
    }

    bool operator!=(base_type other) const {
        return internal_handle_ != other;
    }

    JseHandle<Tag, Invalid>& operator++() {
        ++internal_handle_;
        return *this;
    }

    JseHandle<Tag, Invalid> operator++(int) {
        Handle<Tag, Invalid> tmp{ *this };
        ++internal_handle_;
        return tmp;
    }

    bool isValid() const {
        return internal_handle_ != invalid;
    }
private:
    base_type internal_handle_;
};

template<typename JseHandle>
class JseHandleGenerator {
public:
    JseHandleGenerator() : next_{ 1 } {
    }
    ~JseHandleGenerator() = default;

    JseHandle next() {
        return next_++;
    }

    void release(JseHandle h) {
        //free_.push_back(h);
    }
private:
    JseHandle next_;
    std::vector<JseHandle> free_;
};

namespace std {
    template <typename Tag, int Invalid> struct hash<JseHandle<Tag, Invalid>> {
        typedef JseHandle<Tag, Invalid> argument_type;
        typedef std::size_t result_type;
        result_type operator()(const argument_type& k) const {
            std::hash<typename JseHandle<Tag, Invalid>::base_type> base_hash;
            return base_hash(k.internal());
        }
    };
}

#endif // !JseHANDLE_H