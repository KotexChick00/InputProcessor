#pragma once
#include <pch.h>

namespace CoreEngine::Time {
	class CORE_API Time {
	public:
		// FOR CLIENT
		CORE_FORCE_INLINE float GetDeltaTime() const { return mDeltaTime; }
		CORE_FORCE_INLINE float GetTimeScale() const { return mTimeScale; }
		CORE_FORCE_INLINE float GetTimeNow() const { return mTimeNow; }
		CORE_FORCE_INLINE void SetTimeScale(float timeScale) { mTimeScale = timeScale; }

		// FOR CORE
		CORE_FORCE_INLINE void SetTimeNow(float timeNow) { mTimeNow = timeNow; }
		CORE_FORCE_INLINE void SetDeltaTime(float deltaTime) { mDeltaTime = deltaTime; }

		// Update time scale of the time system in the loop
		void Update();

		static Time* GetInstance() {
			static Time time;
			return &time;
		}

	private:
		float mDeltaTime, mTimeScale = 1.0f, mTimeNow;

		Time() = default;
		~Time() = default;
	};
}