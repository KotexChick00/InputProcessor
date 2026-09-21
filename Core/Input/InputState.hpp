#pragma once
#include <Input/KeyboardInput.hpp>
#include <Input/MouseInput.hpp>

namespace CoreEngine::Input {
	struct CORE_API InputState {
		KeyboardInput* KeyboardInput;
		MouseInput* MouseInput;
	};
}