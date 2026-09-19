#pragma once
#include <Input/IKeyboardInput.hpp>
#include <GLFW/glfw3.h>

namespace CoreEngine::Input::GLFW {
	class GLFWKeyBoardInput : public IKeyboardInput {
	public:
		GLFWKeyBoardInput(GLFWwindow* window);

		bool CheckIsPressed(KeyboardKey key) override;
		bool CheckIsReleased(KeyboardKey key) override;

		KeyState GetKeyState(KeyboardKey key) override;
	private:
		GLFWwindow* mWindow = nullptr;

	private:
		static int ToGLFWKey(KeyboardKey key);
	};
}