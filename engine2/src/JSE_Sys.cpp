#include "JSE.h"

SDL_Window* p_defaultWindow{};

const char* JseGetPlatform() {
    return SDL_GetPlatform();
}

int JseGetCPUCacheLineSize() {
    return SDL_GetCPUCacheLineSize();
}

int JseGetCpuCount() {
    return SDL_GetCPUCount();
}

int JseGetSystemRAM() {
    return SDL_GetSystemRAM();
}

int JseMessageBox(uint32_t flags, const char* title, const char* message) {
    Uint32 _flags{};
    switch(flags) {
        case JseMESSAGEBOX_ERROR:
            _flags = SDL_MESSAGEBOX_ERROR;
            break;
        case JseMESSAGEBOX_WARNING:
            _flags = SDL_MESSAGEBOX_WARNING;
            break;
        case JseMESSAGEBOX_INFO:
            _flags = SDL_MESSAGEBOX_INFORMATION;
            break;
    }

    return SDL_ShowSimpleMessageBox(_flags, title, message, p_defaultWindow);

}