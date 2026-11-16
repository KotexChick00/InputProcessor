#pragma once

#include <variant>
#include <Input/KeyboardInput.hpp>
#include <Input/MouseInput.hpp>

namespace Domain::Camera
{
	enum class MouseScrollDirection
	{
		Up, // Should be Y+
		Down // Should be Y-
	};
	using InputBinding = std::variant<CoreEngine::KeyboardKey, CoreEngine::MouseButton, MouseScrollDirection>;
}