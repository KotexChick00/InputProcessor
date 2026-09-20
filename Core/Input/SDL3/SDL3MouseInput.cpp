#include <Input/SDL3/SDL3MouseInput.h>

namespace CoreEngine::Input::SDL3 {

    namespace {
        inline bool IsButtonDown(Uint32 buttonMask, int sdlButton) {
            return (buttonMask & SDL_BUTTON_MASK(sdlButton)) != 0;
        }
    }

    SDL3MouseInput::SDL3MouseInput(SDL_Window* window) : mWindow(window) {
        if (mWindow) {
            float x = 0.0f, y = 0.0f;
            mCurrButtons = SDL_GetMouseState(&x, &y);
            mPrevButtons = mCurrButtons;
            mPrevX = mCurrX = x;
            mPrevY = mCurrY = y;
        }
    }

    void SDL3MouseInput::Update() {

        mPrevButtons = mCurrButtons;
        mPrevX = mCurrX;
        mPrevY = mCurrY;

        mCurrButtons = SDL_GetMouseState(&mCurrX, &mCurrY);


        mScrollDeltaX = 0.0f;
        mScrollDeltaY = 0.0f;
    }

    bool SDL3MouseInput::CheckIsPressed(MouseButton mouseButton) {
        const int sdlBtn = ToSDL3MouseButton(mouseButton);
        if (sdlBtn == 0) return false;

        const bool curr = IsButtonDown(mCurrButtons, sdlBtn);
        const bool prev = IsButtonDown(mPrevButtons, sdlBtn);
        return curr && !prev;
    }

    bool SDL3MouseInput::CheckIsReleased(MouseButton mouseButton) {
        const int sdlBtn = ToSDL3MouseButton(mouseButton);
        if (sdlBtn == 0) return false;

        const bool curr = IsButtonDown(mCurrButtons, sdlBtn);
        const bool prev = IsButtonDown(mPrevButtons, sdlBtn);
        return !curr && prev;
    }

    bool SDL3MouseInput::CheckIsHeld(MouseButton mouseButton) {
        const int sdlBtn = ToSDL3MouseButton(mouseButton);
        if (sdlBtn == 0) return false;

        return IsButtonDown(mCurrButtons, sdlBtn);
    }

    MouseButtonState SDL3MouseInput::GetMouseButtonState(MouseButton mouseButton) {
        const int sdlBtn = ToSDL3MouseButton(mouseButton);
        if (sdlBtn == 0) return MouseButtonState::None;

        const bool curr = IsButtonDown(mCurrButtons, sdlBtn);
        const bool prev = IsButtonDown(mPrevButtons, sdlBtn);

        if (curr && !prev) return MouseButtonState::Pressed;
        if (curr && prev) return MouseButtonState::Held;
        if (!curr && prev) return MouseButtonState::Released;
        return MouseButtonState::None;
    }


    void SDL3MouseInput::GetPositionCoordinates(float& x, float& y) const {
        x = mCurrX;
        y = mCurrY;
    }

    void SDL3MouseInput::GetPositionDelta(float& deltaX, float& deltaY) const {
        deltaX = mCurrX - mPrevX;
        deltaY = mCurrY - mPrevY;
    }

    float SDL3MouseInput::GetScrollDeltaX() const {
        return mScrollDeltaX;
    }

    float SDL3MouseInput::GetScrollDeltaY() const {
        return mScrollDeltaY;
    }

    void SDL3MouseInput::AddScrollDelta(float x, float y) {
        mScrollDeltaX += x;
        mScrollDeltaY += y;
    }


    int SDL3MouseInput::ToSDL3MouseButton(MouseButton mouseButton) {
        switch (mouseButton) {
        case MouseButton::Button1:  return SDL_BUTTON_LEFT;   
        case MouseButton::Button2:  return SDL_BUTTON_RIGHT;  
        case MouseButton::Button3:  return SDL_BUTTON_MIDDLE; 
        case MouseButton::Button4:  return SDL_BUTTON_X1;     
        case MouseButton::Button5:  return SDL_BUTTON_X2;     
        default:                    return 0; // invalid
        }
    }

} 