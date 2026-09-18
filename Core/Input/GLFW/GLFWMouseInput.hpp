#pragma once
#include <Input/IMouseInput.hpp>
#include <GLFW/glfw3.h>

namespace InputProcessor::Input::GLFW {
	class GLFWMouseInput : public IMouseInput {
	public:
		GLFWMouseInput(GLFWwindow* window);
		~GLFWMouseInput() = default;

		bool CheckIsPressed(MouseButton mouseButton) override;
		bool CheckIsReleased(MouseButton mouseButton) override;
		bool CheckIsScrolled(MouseScroll mouseScroll) override;

	private:
		static int ToGLFWMouseButton(MouseButton mouseButton);
		static int GetMouseDeviceType(); // Placeholder for actual device type detection

		GLFWwindow* mWindow = nullptr;
	};
}