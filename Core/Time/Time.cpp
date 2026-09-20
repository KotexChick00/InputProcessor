#include <Time/Time.hpp>
#include <chrono>

namespace CoreEngine::Time {
    void Time::Update() {
        static auto lastTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();

        std::chrono::duration<float> elapsedTime = currentTime - lastTime;
        lastTime = currentTime;

        mDeltaTime = elapsedTime.count() * mTimeScale;
        mCurrentSecond += mDeltaTime;
    }
}