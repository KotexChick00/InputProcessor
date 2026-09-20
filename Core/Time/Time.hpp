#pragma once
#include <pch.h>

namespace CoreEngine::Time {
	class CORE_API Time {
	public:
		CORE_FORCE_INLINE static float GetDeltaTime() { return sDeltaTime; }
		CORE_FORCE_INLINE static float GetTimeScale() { return sTimeScale; }
		CORE_FORCE_INLINE static float GetCurrentTime() { return sCurrentSecond; }

		CORE_FORCE_INLINE static void SetTimeScale(float timeScale) { sTimeScale = timeScale; }

		// Update time scale of the time system in the loop
		static void Update();

	private:
		static float sDeltaTime, sTimeScale, sCurrentSecond;

		Time() = default;
		~Time() = default;
	};
}