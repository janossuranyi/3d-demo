#ifndef JSE_CORE_H
#define JSE_CORE_H

#include <cinttypes>

class JSE_NonCopyable {
public:
    JSE_NonCopyable() = default;
    JSE_NonCopyable(JSE_NonCopyable&) = delete;
    JSE_NonCopyable& operator=(JSE_NonCopyable&) = delete;
};

class JSE_NonMovable {
public:
    JSE_NonMovable() = default;
    JSE_NonMovable(JSE_NonCopyable&) = delete;
    JSE_NonMovable(JSE_NonCopyable&&) = delete;
    JSE_NonMovable& operator=(JSE_NonMovable&) = delete;
    JSE_NonMovable& operator=(JSE_NonMovable&&) = delete;
};


extern void JSE_Init(int argc, const char** argv);
extern void JSE_Shutdown();
extern uint64_t JSE_GetTicks();

typedef struct JSE_CoreContext {
    int version;    
} JSE_CoreContext;

extern JSE_CoreContext g_CoreContext;

#endif
