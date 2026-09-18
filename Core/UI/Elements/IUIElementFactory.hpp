#pragma once
#include <UI/Elements/IUIElement.hpp>

namespace InputProcessor::UI {
	class IUIElementFactory {
	public:
		virtual IContainer* CreateContainer() = 0;
		virtual IVboxContainer* CreateVboxContainer() = 0;
		virtual IHboxContainer* CreateHboxContainer() = 0;
		virtual IButton* CreateButton() = 0;
	};
}