#include "JSE.h"

void JseInputManager::SetOnExitEvent(const std::function<void()> onexit)
{
	onExit_ = onexit;
}

void JseInputManager::SetOnKeyboardEvent(const std::function<void(JseKeyboardEvent)> onkey)
{
	onKeyboard_ = onkey;
}

void JseInputManager::ProcessEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != SDL_FALSE)
    {
        if (e.type == SDL_QUIT && onExit_)
        {
            onExit_();
        }
        else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP && onKeyboard_)
        {
            onKeyboard_(e.key);
        }
    }
}
