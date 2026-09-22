#include <Window/WindowPlatformFactory.hpp>
#include <pch.h>

#include <Window/GLFW/GLFWWindow.hpp>

namespace CoreEngine::Window {
	IWindow* WindowPlatformFactory::Create(WindowPlatformSpec spec) {
		switch (spec) {
		case WindowPlatformSpec::GLFW: return new GLFW::GLFWWindow();
		case WindowPlatformSpec::SDL: 
			CORE_ASSERT(false && "SDL Platform currently doesn't support");
			break;
		default: 
			CORE_ASSERT(false && "Didn't found the platform");
			break;
		}
	}
}