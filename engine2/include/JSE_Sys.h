#ifndef JSE_SYS_H
#define JSE_SYS_H

#include <SDL.h>
#include <cinttypes>

enum JSE_MessageBoxFlags : uint32_t {
    JSE_MESSAGEBOX_ERROR,
    JSE_MESSAGEBOX_WARNING,
    JSE_MESSAGEBOX_INFO
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
const char* JSE_GetPlatform();

/*
Determine the L1 cache line size of the CPU.
*/
int JSE_GetCPUCacheLineSize();

int JSE_GetCpuCount();
int JSE_GetSystemRAM();
int JSE_MessageBox(uint32_t flags, const char* title, const char* message);

#endif