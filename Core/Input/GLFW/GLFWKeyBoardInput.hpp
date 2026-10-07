#pragma once
#include <Input/KeyboardInput.hpp>
#ifdef _USE_GLAD_OPENGL
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

namespace CoreEngine::Input::GLFW {
	class GLFWKeyBoardInput : public KeyboardInput {
	public:
		GLFWKeyBoardInput(GLFWwindow* window);

		bool CheckIsPressed(KeyboardKey key) override;
		bool CheckIsReleased(KeyboardKey key) override;
		bool CheckIsJustPressed(KeyboardKey key) override;

		KeyState GetKeyState(KeyboardKey key) override;

		void Update();

	private:
		GLFWwindow* mWindow = nullptr;
		std::unordered_map<KeyboardKey, bool> mWasPressed;

	private:
		static int ToGLFWKey(KeyboardKey key);
	};
}