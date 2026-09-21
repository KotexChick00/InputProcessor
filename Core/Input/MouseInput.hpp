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
		Released
	};

	class CORE_API MouseInput {
	public:
		virtual ~MouseInput() = default;

		virtual bool CheckIsPressed(MouseButton mouseButton) = 0;
		virtual bool CheckIsReleased(MouseButton mouseButton) = 0;
		virtual bool CheckIsJustPressed(MouseButton mouseButton) = 0;

		virtual MouseButtonState GetMouseButtonState(MouseButton mouseButton) = 0;

		CORE_FORCE_INLINE float GetPositionX() const { return mXPos; }
		CORE_FORCE_INLINE float GetPositionY() const { return mYPos; }
		CORE_FORCE_INLINE float GetDeltaX() const { return mDeltaX; }
		CORE_FORCE_INLINE float GetDeltaY() const { return mDeltaY; }
		CORE_FORCE_INLINE float GetScrollX() const { return mScrollX; }
		CORE_FORCE_INLINE float GetScrollY() const { return mScrollY; }

		CORE_FORCE_INLINE void SetPosition(float x, float y) { mXPos = x; mYPos = y; }
		CORE_FORCE_INLINE void SetPositionX(float x) { mXPos = x; }
		CORE_FORCE_INLINE void SetPositionY(float y) { mYPos = y; }
		
		CORE_FORCE_INLINE void SetDelta(float dx, float dy) { mDeltaX = dx; mDeltaY = dy; }
		CORE_FORCE_INLINE void SetDeltaX(float dx) { mDeltaX = dx; }
		CORE_FORCE_INLINE void SetDeltaY(float dy) { mDeltaY = dy; }

		CORE_FORCE_INLINE void SetScroll(float scrollX, float scrollY) { mScrollX = scrollX; mScrollY = scrollY; }
		CORE_FORCE_INLINE void SetScrollX(float scrollX) { mScrollX = scrollX; }
		CORE_FORCE_INLINE void SetScrollY(float scrollY) { mScrollY = scrollY; }

	private:
		float mXPos, mYPos, mDeltaX, mDeltaY, mScrollX, mScrollY;

	protected:
		// Helpers
		inline static std::vector<MouseButton> sMouseButtons = {
			MouseButton::Button1, // Left
			MouseButton::Button2, // Right
			MouseButton::Button3, // Middle
			MouseButton::Button4, // Thumb back or Thumb button 1
			MouseButton::Button5, // Thumb forward or Thumb button 2
			MouseButton::Button6,
			MouseButton::Button7,
			MouseButton::Button8
		};
	};

	class IMouseInputCheckIsHeldable {
	public:
		virtual bool CheckIsHeld(MouseButton mouseButton) = 0;
	};
}