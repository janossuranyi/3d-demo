#include "JSE.h"

JSE_CoreContext g_CoreContext {100};


void JSE_Init(int argc, const char** argv) {
	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
}

void JSE_Shutdown() {
	SDL_Quit();
}

uint64_t JSE_GetTicks() {
	return SDL_GetTicks64();
}