#include <Window/WindowPlatformFactory.hpp>
#include <pch.h>

#include <Window/GLFW/GLFWWindow.hpp>

namespace CoreEngine::Window {
	IWindow* WindowPlatformFactory::Create(WindowPlatformSpec spec) {
		switch (spec) {
		case WindowPlatformSpec::GLFW: return new GLFW::GLFWWindow();
		case WindowPlatformSpec::SDL: CORE_ASSERT("SDL Platform currently doesn't support");
		default: CORE_ASSERT("Didn't found the platform");
		}
	}
}