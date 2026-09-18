#pragma once

#include <chrono>

namespace InputProcessor::Window {
	class FrameLimiter {
	public:
		explicit FrameLimiter(unsigned int targetFPS = 0);

		void SetTargetFPS(unsigned int targetFPS);
		unsigned int GetTargetFPS() const;

		void BeginFrame();
		void EndFrame();

	private:
		using Clock = std::chrono::steady_clock;

		unsigned int mTargetFPS = 0;
		Clock::time_point mFrameStartTime{};
		bool mHasFrameStart = false;
	};
}
