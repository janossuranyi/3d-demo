#ifndef JseCORE_H
#define JseCORE_H

#include <cinttypes>

enum class JseResult {
	SUCCESS = 0,
	GENERIC_ERROR = 1,
	OUT_OF_MEMORY_ERROR = 0x10000,
	NOT_IMPLEMENTED = 0x10001,
    ALREADY_EXISTS = 0x10002,
    NOT_EXISTS = 0x10003,
    INTERVAL_EXCEEDED = 0x10004,
	INVALID_SURFACE_DIMENSION = 0x20000
};

class JseNonCopyable {
public:
    JseNonCopyable() = default;
    JseNonCopyable(JseNonCopyable&) = delete;
    JseNonCopyable& operator=(JseNonCopyable&) = delete;
};

class JseNonMovable {
public:
    JseNonMovable() = default;
    JseNonMovable(JseNonCopyable&) = delete;
    JseNonMovable(JseNonCopyable&&) = delete;
    JseNonMovable& operator=(JseNonMovable&) = delete;
    JseNonMovable& operator=(JseNonMovable&&) = delete;
};


extern void JseInit(int argc, char** argv);
extern void JseShutdown();
extern uint64_t JseGetTicks();

typedef struct JseCoreContext {
    int version;    
} JseCoreContext;

extern JseCoreContext g_CoreContext;

#endif
