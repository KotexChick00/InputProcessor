#pragma once
#include <Input/KeyboardInput.hpp>
#include <Input/MouseInput.hpp>

namespace CoreEngine
{
    struct CORE_API InputState
    {
        CoreEngine::KeyboardInput* KeyboardInput = nullptr;
        CoreEngine::MouseInput* MouseInput = nullptr;
    };
}
