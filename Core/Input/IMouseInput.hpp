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
		virtual bool CheckIsHeld(MouseButton mouseButton) = 0;

		virtual MouseButtonState GetMouseButtonState(MouseButton mouseButton) = 0;

		virtual float GetScrollDeltaX() const = 0;
		virtual float GetScrollDeltaY() const = 0;

		virtual void GetPositionCoordinates(float& x, float& y) const = 0;
		virtual void GetPositionDelta(float& deltaX, float& deltaY) const = 0;
	};
}