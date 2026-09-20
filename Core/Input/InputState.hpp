#pragma once
#include <Input/IKeyboardInput.hpp>
#include <Input/IMouseInput.hpp>

namespace CoreEngine::Input {
	struct CORE_API InputState {
		IKeyboardInput* KeyboardInput;
		IMouseInput* MouseInput;

		InputState(IKeyboardInput* keyboardInput, IMouseInput* mouseInput) : KeyboardInput(keyboardInput), MouseInput(mouseInput) { }
	};
}