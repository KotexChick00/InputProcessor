#pragma once
#include <iostream>

namespace InputProcessor::Input {
	enum class MouseButton {
		Button1, // Left
		Button2, // Right
		Button3, // Middle
		Button4,
		Button5,
		Button6,
		Button7,
		Button8,
		ButtonLast = Button8,
		ButtonLeft = Button1,
		ButtonRight = Button2,
		ButtonMiddle = Button3
	};

	enum class MouseButtonState {
		None,
		Pressed,
		Held,
		Released
	};

	class IMouseInput {
	public:
		virtual ~IMouseInput() = default;

		virtual bool CheckIsPressed(MouseButton mouseButton) = 0;
		virtual bool CheckIsReleased(MouseButton mouseButton) = 0;
	};

	class IMouseInputCheckIsHeldable {
	public:
		virtual bool CheckIsHeld(MouseButton mouseButton) = 0;
	};
}