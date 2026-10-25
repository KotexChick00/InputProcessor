#pragma once
#include <Input/MouseInput.hpp>
#ifdef _USE_GLAD_OPENGL
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

namespace CoreEngine::Input::GLFW {
	class GLFWMouseInput : public MouseInput {
	public:
		GLFWMouseInput(GLFWwindow* window);
		~GLFWMouseInput() = default;

		bool CheckIsPressed(MouseButton mouseButton) override;
		bool CheckIsReleased(MouseButton mouseButton) override;
		bool CheckIsJustPressed(MouseButton mouseButton) override;

		MouseButtonState GetMouseButtonState(MouseButton mouseButton) override;

		void Update();

	private:
		std::unordered_map<MouseButton, bool> sMouseButtons;
		static int ToGLFWMouseButton(MouseButton mouseButton);

		GLFWwindow* mWindow = nullptr;
	};
}