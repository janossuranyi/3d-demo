#ifndef JSE_SYS_H
#define JSE_SYS_H

#include <SDL.h>
#include <cinttypes>

enum JseMessageBoxFlags : uint32_t {
    JseMESSAGEBOX_ERROR,
    JseMESSAGEBOX_WARNING,
    JseMESSAGEBOX_INFO
};

extern SDL_Window* p_defaultWindow;

/*
Here are the names returned for some (but not all) supported platforms:

    "Windows"
    "Mac OS X"
    "Linux"
    "iOS"
    "Android"
*/
const char* JseGetPlatform();

/*
Determine the L1 cache line size of the CPU.
*/
int JseGetCPUCacheLineSize();

int JseGetCpuCount();
int JseGetSystemRAM();
int JseMessageBox(uint32_t flags, const char* title, const char* message);

#endif