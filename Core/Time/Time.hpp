#pragma once
#include <pch.h>

namespace CoreEngine::Time {
	class CORE_API Time {
	public:
		CORE_FORCE_INLINE float GetDeltaTime() const { return mDeltaTime; }
		CORE_FORCE_INLINE float GetTimeScale() const { return mTimeScale; }
		CORE_FORCE_INLINE float GetCurrentTime() const { return mCurrentSecond; }

		CORE_FORCE_INLINE void SetTimeScale(float timeScale) { mTimeScale = timeScale; }

		// Update time scale of the time system in the loop
		void Update();

		static Time* GetInstance() {
			static Time time;
			return &time;
		}

	private:
		float mDeltaTime, mTimeScale, mCurrentSecond;

		Time() = default;
		~Time() = default;
	};
}