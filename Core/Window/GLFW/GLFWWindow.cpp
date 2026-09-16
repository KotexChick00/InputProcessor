#include <Window/GLFW/GLFWWindow.hpp>
#include <Logger/Logger.hpp>

namespace InputProcessor::Window::GLFW {
	using namespace InputProcessor::Logger;

	void GLFWWindow::Init(const WindowConfiguration& config) {
		if (!glfwInit()) {
			IP_ENGINE_CRITICAL("Failed to initialize GLFW.");
			return;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

		mWindow = glfwCreateWindow(
			config.Width,
			config.Height,
			config.Title.c_str(),
			nullptr,
			nullptr
		);

		if (!mWindow) {
			IP_ENGINE_CRITICAL("Failed to create GLFW window: '{}' ({}x{})", config.Title, config.Width, config.Height);
			glfwTerminate();
			return;
		}

		glfwMakeContextCurrent(mWindow);

		if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
			IP_ENGINE_CRITICAL("Failed to initialize OpenGL loader (GLAD).");
			Close();
			return;
		}

		IP_ENGINE_TRACE("Window initialized successfully: '{}' ({}x{})", config.Title, config.Width, config.Height);
	}

	void GLFWWindow::PollEvents() {
		glfwPollEvents();
	}

	void GLFWWindow::SwapBuffers() {
		if (mWindow) {
			glfwSwapBuffers(mWindow);
		}
	}

	bool GLFWWindow::CheckShouldClose() {
		return mWindow ? glfwWindowShouldClose(mWindow) : true;
	}

	void GLFWWindow::Close() {
		if (mWindow) {
			glfwDestroyWindow(mWindow);
			mWindow = nullptr;
			glfwTerminate();
			IP_ENGINE_TRACE("Window closed and GLFW terminated.");
		}
	}
}