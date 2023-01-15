#ifndef JSE_HANDLE_H
#define JSE_HANDLE_H

#include <cinttypes>
#include <limits>
#include <vector>

template<typename Tag, int Invalid>
class JSE_Handle {
public:
    using base_type = uint32_t;
    static const base_type invalid = static_cast<base_type>(Invalid);

    JSE_Handle() : internal_handle_(Invalid) {
    }

    explicit JSE_Handle(base_type ahandle) : internal_handle_(ahandle) {
    }

    explicit operator base_type() const {
        return internal();
    }

    base_type internal() const {
        return internal_handle_;
    }

    JSE_Handle<Tag, Invalid>& operator=(base_type other) {
        internal_handle_ = other;
        return *this;
    }

    bool operator==(JSE_Handle<Tag, Invalid>& other) const {
        return internal_handle_ == other.internal_handle_;
    }

    bool operator==(const JSE_Handle<Tag, Invalid>& other) const {
        return internal_handle_ == other.internal_handle_;
    }

    bool operator==(base_type other) const {
        return internal_handle_ == other;
    }

    bool operator!=(JSE_Handle<Tag, Invalid>& other) const {
        return internal_handle_ != other.internal_handle_;
    }

    bool operator!=(const JSE_Handle<Tag, Invalid>& other) const {
        return internal_handle_ != other.internal_handle_;
    }

    bool operator!=(base_type other) const {
        return internal_handle_ != other;
    }

    JSE_Handle<Tag, Invalid>& operator++() {
        ++internal_handle_;
        return *this;
    }

    JSE_Handle<Tag, Invalid> operator++(int) {
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

template<typename JSE_Handle>
class JSE_HandleGenerator {
public:
    JSE_HandleGenerator() : next_{ 1 } {
    }
    ~JSE_HandleGenerator() = default;

    JSE_Handle next() {
        return next_++;
    }

    void release(JSE_Handle h) {
        //free_.push_back(h);
    }
private:
    JSE_Handle next_;
    std::vector<JSE_Handle> free_;
};

namespace std {
    template <typename Tag, int Invalid> struct hash<JSE_Handle<Tag, Invalid>> {
        typedef JSE_Handle<Tag, Invalid> argument_type;
        typedef std::size_t result_type;
        result_type operator()(const argument_type& k) const {
            std::hash<typename JSE_Handle<Tag, Invalid>::base_type> base_hash;
            return base_hash(k.internal());
        }
    };
}

#endif // !JSE_HANDLE_H