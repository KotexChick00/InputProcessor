#pragma once
#include <array>
#include <Input/IKeyboardInput.hpp>
#include <SDL3/SDL.h>

namespace InputProcessor::Input::SDL3 {
	class SDL3KeyBoardInput : public IKeyboardInput {
	public:
		explicit SDL3KeyBoardInput(SDL_Window* window);

		void Update();   // gọi 1 lần mỗi frame, sau PollEvents

		bool CheckIsPressed(KeyboardKey key) override;
		bool CheckIsReleased(KeyboardKey key) override;
		bool CheckIsHeld(KeyboardKey key); 
		KeyState GetKeyState(KeyboardKey key) override;

		KeyState GetKeyState(KeyboardKey key) override;

	private:
		SDL_Window* mWindow = nullptr;
		std::array<bool, SDL_SCANCODE_COUNT> mPrev{};
		std::array<bool, SDL_SCANCODE_COUNT> mCurr{};
		static SDL_Scancode ToSDL3Scancode(KeyboardKey key);   // đổi tên + đổi kiểu trả về
	};
}