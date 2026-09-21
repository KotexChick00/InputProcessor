#include <pch.h>
#include <Window/WindowPlatformFactory.hpp>


#include <Window/GLFW/GLFWWindow.hpp>
#include <Window/SDL3/SDL3Window.hpp>

namespace CoreEngine::Window {
	IWindow* WindowPlatformFactory::Create(WindowPlatformSpec spec) {
		switch (spec) {
		case WindowPlatformSpec::GLFW: return new GLFW::GLFWWindow();
		case WindowPlatformSpec::SDL: return new SDL3::SDL3Window();
		}
		CORE_ASSERT(false, "Didn't found the platform");
		return nullptr;
	}
}