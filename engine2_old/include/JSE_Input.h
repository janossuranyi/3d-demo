#ifndef JSE_INPUT_H
#define JSE_INPUT_H

using JseKeyCode = SDL_KeyCode;
using JseKeyMod = SDL_Keymod;
using JseKeyboardEvent = SDL_KeyboardEvent;
using JseMouseMotionEvent = SDL_MouseMotionEvent;
using JseMouseButtonEvent = SDL_MouseButtonEvent;
using JseMouseWheelEvent = SDL_MouseWheelEvent;
using JseEvent = SDL_Event;

#define JSE_RELEASED SDL_RELEASED
#define JSE_PRESSED SDL_PRESSED
namespace js {
	class InputManager : public js::Module
	{
	private:
		std::function<void(JseKeyboardEvent)> onKeyboard_;
		std::function<void()> onExit_;
	public:

		void SetOnExitEvent(const std::function<void()> onexit);
		void SetOnKeyboardEvent(const std::function<void(JseKeyboardEvent)> onkey);
		void ProcessEvents();

		virtual ~InputManager() {}

		// Inherited via JseModule
		virtual JsType typeIndex() const override;
	};
}
#endif