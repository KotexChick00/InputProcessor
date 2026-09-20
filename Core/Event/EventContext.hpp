#pragma once
#include <pch.h>

namespace CoreEngine::Event {
	// Note: Marker interface
	class CORE_API EventContext {
	public:
		EventContext() = default;
		virtual ~EventContext() = default;
	};
};