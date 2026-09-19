#pragma once
#include <Input/IKeyboardInput.hpp>
#include <Input/IMouseInput.hpp>

namespace InputProcessor::Input {
	struct InputState {
		IKeyboardInput* KeyboardInput;
		IMouseInput* MouseInput;
	};
}