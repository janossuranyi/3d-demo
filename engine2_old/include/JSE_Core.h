#ifndef JSE_CORE_H
#define JSE_CORE_H

#include <cinttypes>

namespace js {
    enum class Result {
        SUCCESS = 0,
        GENERIC_ERROR = 1,
        OUT_OF_MEMORY_ERROR = 0x10000,
        NOT_IMPLEMENTED = 0x10001,
        ALREADY_EXISTS = 0x10002,
        NOT_EXISTS = 0x10003,
        INVALID_VALUE = 0x10004,
        INTERVAL_EXCEEDED = 0x10004,
        FRAMEBUFFER_INCOMPLETE = 0x10005,
        INVALID_SURFACE_DIMENSION = 0x20000
    };

    class NonCopyable {
    public:
        NonCopyable() = default;
        NonCopyable(NonCopyable&) = delete;
        NonCopyable& operator=(NonCopyable&) = delete;
    };

    class NonMovable {
    public:
        NonMovable() = default;
        NonMovable(NonCopyable&) = delete;
        NonMovable(NonCopyable&&) = delete;
        NonMovable& operator=(NonMovable&) = delete;
        NonMovable& operator=(NonMovable&&) = delete;
    };

}

extern void JseInit(int argc, char** argv);
extern void JseShutdown();
extern uint64_t JseGetTicks();

typedef struct JseCoreContext {
    int version;    
} JseCoreContext;

extern JseCoreContext g_CoreContext;

#endif
