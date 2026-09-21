#pragma once
#include <pch.h>

namespace CoreEngine::Input {
	enum class CORE_API MouseButton {
		Button1, // Left
		Button2, // Right
		Button3, // Middle
		Button4, // Thumb back or Thumb button 1
		Button5, // Thumb forward or Thumb button 2
		Button6, 
		Button7,
		Button8,
		ButtonLast = Button8,
		ButtonLeft = Button1,
		ButtonRight = Button2,
		ButtonMiddle = Button3
	};

	enum class CORE_API MouseButtonState {
		None,
		Pressed,
		Released,
		Held
	};

	class CORE_API IMouseInput {
	public:
		virtual ~IMouseInput() = default;

		virtual bool CheckIsPressed(MouseButton mouseButton) = 0;
		virtual bool CheckIsReleased(MouseButton mouseButton) = 0;
	};
}