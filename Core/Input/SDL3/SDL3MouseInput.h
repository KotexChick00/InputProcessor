#pragma once
#include <Input/IMouseInput.hpp>
#include <SDL3/SDL.h>

namespace CoreEngine::Input::SDL3 {

    class SDL3MouseInput : public IMouseInput {
    public:
        explicit SDL3MouseInput(SDL_Window* window);
        ~SDL3MouseInput() = default;

        void Update();

        // Buttons
        bool CheckIsPressed(MouseButton mouseButton) override;
        bool CheckIsReleased(MouseButton mouseButton) override;
        bool CheckIsHeld(MouseButton mouseButton) override;
        MouseButtonState GetMouseButtonState(MouseButton mouseButton) override;

        // Scroll
        float GetScrollDeltaX() const override;
        float GetScrollDeltaY() const override;

        // Position
        void GetPositionCoordinates(float& x, float& y) const override;
        void GetPositionDelta(float& deltaX, float& deltaY) const override;

        // Gọi từ event loop khi nhận SDL_EVENT_MOUSE_WHEEL
        void AddScrollDelta(float x, float y);

    private:
        SDL_Window* mWindow = nullptr;

        // Button bitmask (SDL_GetMouseState trả về Uint32)
        Uint32 mPrevButtons = 0;
        Uint32 mCurrButtons = 0;

        // Position
        float mPrevX = 0.0f;
        float mPrevY = 0.0f;
        float mCurrX = 0.0f;
        float mCurrY = 0.0f;

        // Scroll (cộng dồn từ event, reset mỗi Update)
        float mScrollDeltaX = 0.0f;
        float mScrollDeltaY = 0.0f;

        static int ToSDL3MouseButton(MouseButton mouseButton);
    };

} 