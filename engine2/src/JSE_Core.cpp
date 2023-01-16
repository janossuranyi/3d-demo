#include "JSE.h"

JseCoreContext g_CoreContext {100};


void JseInit(int argc, char** argv) {

	int err;
	if ((err = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0)) {
		Error("ERROR: %s", SDL_GetError());
	}
}

void JseShutdown() {
	SDL_Quit();
}

uint64_t JseGetTicks() {
	return SDL_GetTicks64();
}