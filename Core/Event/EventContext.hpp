#pragma once
#include <iostream>

namespace CoreEngine::Event {
	// Note: Marker interface
	class EventContext {
	public:
		virtual ~EventContext() = 0;
	};
};