#include <Input/GLFW/GLFWMouseInput.hpp>

namespace InputProcessor::Input::GLFW {

	GLFWMouseInput::GLFWMouseInput(GLFWwindow* window) : mWindow(window) { }

	bool GLFWMouseInput::CheckIsPressed(MouseButton mouseButton) {
		if (mWindow == nullptr) return false;
		int glfwBtn = ToGLFWMouseButton(mouseButton);
		int state = glfwGetMouseButton(mWindow, glfwBtn);
		return state == GLFW_PRESS;
	}

	bool GLFWMouseInput::CheckIsReleased(MouseButton mouseButton) {
		if (mWindow == nullptr) return false;
		int glfwBtn = ToGLFWMouseButton(mouseButton);
		int state = glfwGetMouseButton(mWindow, glfwBtn);
		return state == GLFW_RELEASE;
	}

	bool GLFWMouseInput::CheckIsScrolled(MouseScroll mouseScroll) {
		// GLFW does not provide a direct way to check if a mouse button is scrolled.
		// Scrolling is typically handled via scroll callbacks, so this function may not be applicable.
		return false;
	}

	int GLFWMouseInput::ToGLFWMouseButton(MouseButton mouseButton) {
		switch (mouseButton) {
		case MouseButton::Button1: return GLFW_MOUSE_BUTTON_1;
		case MouseButton::Button2: return GLFW_MOUSE_BUTTON_2;
		case MouseButton::Button3: return GLFW_MOUSE_BUTTON_3;
		case MouseButton::Button4: return GLFW_MOUSE_BUTTON_4;
		case MouseButton::Button5: return GLFW_MOUSE_BUTTON_5;
		case MouseButton::Button6: return GLFW_MOUSE_BUTTON_6;
		case MouseButton::Button7: return GLFW_MOUSE_BUTTON_7;
		case MouseButton::Button8: return GLFW_MOUSE_BUTTON_8;
		//case MouseButton::ButtonLeft: return GLFW_MOUSE_BUTTON_LEFT;
		//case MouseButton::ButtonRight: return GLFW_MOUSE_BUTTON_RIGHT;
		//case MouseButton::ButtonMiddle: return GLFW_MOUSE_BUTTON_MIDDLE;
		//case MouseButton::ButtonLast: return GLFW_MOUSE_BUTTON_LAST;
		default: return GLFW_MOUSE_BUTTON_LAST;
		}
	}

	/*
	* Y tuong o day la dung delta cua scroll callback de nhan biet loai chuot va huong cuon.
	* Vi trong GLFW, khong co cach de lay thong tin ve loai chuot (mouse device type) truc tiep,
	* nen cach nay chi mang tinh phong doan va co the co sai sot.
	* Doi voi con lan, gia tri delta thuong la so nguyen, neu cuon len thi delta > 0, cuon xuong thi delta < 0.
	* Doi voi ban di chuot, gia tri delta neu dung thao tac cuon thuong la so thuc, co gia tri tuyet doi < 1,
	* neu cuon len thi delta > 0, cuon xuong thi delta < 0.
	* Tuy nhien, neu dung thao tac phong to thu nho tren touchpad, delta luon la 1 neu phong to
	* va -1 neu thu nho, nen khong the phan biet duoc loai chuot.
	*/

	int GLFWMouseInput::GetMouseDeviceType() {
		// This is a placeholder implementation. In a real scenario, you would query the actual device type.
		return static_cast<int>(MouseDeviceType::Mouse);
	}

}
