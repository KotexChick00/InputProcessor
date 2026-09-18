#pragma once
#include <iostream>

namespace InputProcessor::Event {
	// Note: Marker interface
	class EventContext {
	public:
		virtual ~EventContext() = 0;
	};
};