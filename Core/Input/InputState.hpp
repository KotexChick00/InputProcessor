#pragma once
#include <Input/IKeyboardInput.hpp>
#include <Input/IMouseInput.hpp>

namespace CoreEngine::Input {
	struct InputState {
		IKeyboardInput* KeyboardInput;
		IMouseInput* MouseInput;
	};
}