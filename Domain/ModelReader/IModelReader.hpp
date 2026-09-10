#pragma once
#include <iostream>
#include <IRenderable.hpp>

namespace InputProcessor::Domain {
	class IModelReader {
	public:
		virtual IRenderable* ReadModel(const char* file) = 0;
	};
}