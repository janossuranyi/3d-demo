#ifndef JSE_INPUT_H
#define JSE_INPUT_H

enum JseKeyEventType {
	JSE_KEYDOWN,JSE_KEYUP
};
enum JseKeyState {
	JSE_PRESSED, JSE_RELEASED
};

using JseKeyCode = int;

struct JseKeySym {
	JseKeyCode key;
	uint16_t mod;
};

struct JseKeyEvent {
	JseKeyEventType type;
	JseKeyState state;
	uint32_t timestamp;
	bool repeat;
	JseKeySym keysysm;
};

struct JseMouseButtonEvent {

};
struct JseMouseMoveEvent {};

#endif