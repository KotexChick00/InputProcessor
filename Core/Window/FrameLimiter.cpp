#include <Window/FrameLimiter.hpp>

#include <thread>

namespace InputProcessor::Window {
	FrameLimiter::FrameLimiter(unsigned int targetFPS) {
		SetTargetFPS(targetFPS);
	}

	void FrameLimiter::SetTargetFPS(unsigned int targetFPS) {
		mTargetFPS = targetFPS;
	}

	unsigned int FrameLimiter::GetTargetFPS() const {
		return mTargetFPS;
	}

	void FrameLimiter::BeginFrame() {
		mFrameStartTime = Clock::now();
		mHasFrameStart = true;
	}

	void FrameLimiter::EndFrame() {
		if (!mHasFrameStart || mTargetFPS == 0) {
			mHasFrameStart = false;
			return;
		}

		const auto targetFrameDuration = std::chrono::duration_cast<Clock::duration>(
			std::chrono::duration<double>(1.0 / static_cast<double>(mTargetFPS))
		);

		if (targetFrameDuration <= Clock::duration::zero()) {
			mHasFrameStart = false;
			return;
		}

		const auto elapsedFrameDuration = Clock::now() - mFrameStartTime;
		if (elapsedFrameDuration < targetFrameDuration) {
			std::this_thread::sleep_for(targetFrameDuration - elapsedFrameDuration);
		}

		mHasFrameStart = false;
	}
}
