#include <Time/Time.hpp>
#include <chrono>

namespace CoreEngine::Time {

    float Time::sDeltaTime = 0.0f;
    float Time::sTimeScale = 1.0f;
    float Time::sCurrentSecond = 0.0f;

    void Time::Update() {
        static auto lastTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();

        std::chrono::duration<float> elapsedTime = currentTime - lastTime;
        lastTime = currentTime;

        sDeltaTime = elapsedTime.count() * sTimeScale;
        sCurrentSecond += sDeltaTime;
    }
}