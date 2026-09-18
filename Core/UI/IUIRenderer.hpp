#pragma once
#include <Window/IWindow.hpp>

namespace InputProcessor::UI {
	class IUIRenderer {
	public:
		virtual void Init() = 0;
		virtual void Render() = 0;
		virtual void Free() = 0;
	};
}