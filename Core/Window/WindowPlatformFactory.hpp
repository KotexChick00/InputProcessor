#pragma once
#include <Window/IWindow.hpp>

namespace CoreEngine::Window {
	enum class WindowPlatformSpec {
		GLFW,
		SDL
	};

	class WindowPlatformFactory {
	public:
		static IWindow* Create(WindowPlatformSpec spec);
	private:
		WindowPlatformFactory() = default;
		~WindowPlatformFactory() = default;
		WindowPlatformFactory(WindowPlatformFactory&) = default;
	};
}