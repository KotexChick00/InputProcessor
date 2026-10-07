#pragma once
#include <Input/KeyboardInput.hpp>
#include <Input/MouseInput.hpp>

namespace CoreEngine::Input {
	struct CORE_API InputState {
		CoreEngine::Input::KeyboardInput* KeyboardInput = nullptr;
		CoreEngine::Input::MouseInput* MouseInput = nullptr;
	};
}
