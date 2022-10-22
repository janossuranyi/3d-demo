#pragma once

#include <cinttypes>

namespace gfx {

    template<typename Tag, int Invalid>
    class Handle {
    public:
        using base_type = uint16_t;
        static const base_type invalid = static_cast<base_type>(Invalid);

        Handle() : internal_handle_(Invalid) {
        }

        explicit Handle(base_type ahandle) : internal_handle_(ahandle) {
        }

        explicit operator base_type() const {
            return internal();
        }

        base_type internal() const {
            return internal_handle_;
        }

        Handle<Tag, Invalid>& operator=(base_type other) {
            internal_handle_ = other;
            return *this;
        }

        bool operator==(Handle<Tag, Invalid>& other) const {
            return internal_handle_ == other.internal_handle_;
        }

        bool operator==(const Handle<Tag, Invalid>& other) const {
            return internal_handle_ == other.internal_handle_;
        }

        bool operator==(base_type other) const {
            return internal_handle_ == other;
        }

        bool operator!=(Handle<Tag, Invalid>& other) const {
            return internal_handle_ != other.internal_handle_;
        }

        bool operator!=(const Handle<Tag, Invalid>& other) const {
            return internal_handle_ != other.internal_handle_;
        }

        bool operator!=(base_type other) const {
            return internal_handle_ != other;
        }

        Handle<Tag, Invalid>& operator++() {
            ++internal_handle_;
            return *this;
        }

        Handle<Tag, Invalid> operator++(int) {
            Handle<Tag, Invalid> tmp{ *this };
            ++internal_handle_;
            return tmp;
        }

        bool isValid() const {
            return internal_handle_ != Invalid;
        }
    private:
        base_type internal_handle_;
    };

    template<typename Handle>
    class HandleGenerator {
    public:
        HandleGenerator() : next_{1} {
        }
        ~HandleGenerator() = default;

        Handle next() {

            if (free_.empty())
                return next_++;

            Handle r = free_.back();
            free_.pop_back();

            return r;
        }

        void release(Handle h) {
            free_.push_back(h);
        }
    private:
        Handle next_;
        std::vector<Handle> free_;
    };

}

namespace std {
    template <typename Tag, int Invalid> struct hash<gfx::Handle<Tag, Invalid>> {
        typedef gfx::Handle<Tag, Invalid> argument_type;
        typedef std::size_t result_type;
        result_type operator()(const argument_type& k) const {
            std::hash<typename gfx::Handle<Tag, Invalid>::base_type> base_hash;
            return base_hash(k.internal());
        }
    };
}  // namespace std