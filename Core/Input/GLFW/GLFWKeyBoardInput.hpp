#pragma once
#include <Input/IKeyboardInput.hpp>
#include <GLFW/glfw3.h>

namespace InputProcessor::Input::GLFW {
	class GLFWKeyBoardInput : public IKeyboardInput {
	public:
		GLFWKeyBoardInput(GLFWwindow* window);

		bool CheckIsPressed(Key key) override;
		bool CheckIsReleased(Key key) override;

		KeyState GetKeyState(Key key) override;
	private:
		GLFWwindow* mWindow = nullptr;

	private:
		static int ToGLFWKey(Key key);
	};
}