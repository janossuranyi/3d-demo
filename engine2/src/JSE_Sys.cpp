#include "JSE.h"

SDL_Window* p_defaultWindow{};

const char* JSE_GetPlatform() {
    return SDL_GetPlatform();
}

int JSE_GetCPUCacheLineSize() {
    return SDL_GetCPUCacheLineSize();
}

int JSE_GetCpuCount() {
    return SDL_GetCPUCount();
}

int JSE_GetSystemRAM() {
    return SDL_GetSystemRAM();
}

int JSE_MessageBox(uint32_t flags, const char* title, const char* message) {
    Uint32 _flags{};
    switch(flags) {
        case JSE_MESSAGEBOX_ERROR:
            _flags = SDL_MESSAGEBOX_ERROR;
            break;
        case JSE_MESSAGEBOX_WARNING:
            _flags = SDL_MESSAGEBOX_WARNING;
            break;
        case JSE_MESSAGEBOX_INFO:
            _flags = SDL_MESSAGEBOX_INFORMATION;
            break;
    }

    return SDL_ShowSimpleMessageBox(_flags, title, message, p_defaultWindow);

}