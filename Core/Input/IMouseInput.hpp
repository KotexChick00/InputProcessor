#pragma once
#include <iostream>

namespace InputProcessor::Input {
	enum class MouseButton {
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

	enum class MouseButtonState {
		None,
		Pressed,
		Held,
		Released
	};

	enum class MouseScroll {
		ScrollNeutral,
		ScrollUp,
		ScrollDown,
		ScrollLeft,
		ScrollRight
	};
	
	enum class MouseDeviceType {
		Mouse,
		Touchpad,
		Unknown
	};



	class IMouseInput {
	public:
		virtual ~IMouseInput() = default;

		virtual bool CheckIsPressed(MouseButton mouseButton) = 0;
		virtual bool CheckIsReleased(MouseButton mouseButton) = 0;
		virtual bool CheckIsScrolled(MouseScroll mouseScroll) = 0;
		virtual int GetMouseDeviceType() {
			// This is a placeholder implementation. In a real scenario, you would query the actual device type.
			return static_cast<int>(MouseDeviceType::Mouse);
		}
	};

	class IMouseInputCheckIsHeldable {
	public:
		virtual bool CheckIsHeld(MouseButton mouseButton) = 0;
	};
}