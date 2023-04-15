#include "./System.h"
#include <SDL.h>

namespace jsr {
    const char* GetPlatform() {
        return SDL_GetPlatform();
    }

    int GetCPUCacheLineSize() {
        return SDL_GetCPUCacheLineSize();
    }

    int GetCpuCount() {
        return SDL_GetCPUCount();
    }

    int GetSystemRAM() {
        return SDL_GetSystemRAM();
    }

    int MessageBox(unsigned int flags, const char* title, const char* message) {
        Uint32 _flags{};
        switch (flags) {
        case MESSAGEBOX_ERROR:
            _flags = SDL_MESSAGEBOX_ERROR;
            break;
        case MESSAGEBOX_WARNING:
            _flags = SDL_MESSAGEBOX_WARNING;
            break;
        case MESSAGEBOX_INFO:
            _flags = SDL_MESSAGEBOX_INFORMATION;
            break;
        }

        return SDL_ShowSimpleMessageBox(_flags, title, message, NULL);

    }
    uint64 GetTimeMillisecond()
    {
        return SDL_GetTicks64();
    }
}