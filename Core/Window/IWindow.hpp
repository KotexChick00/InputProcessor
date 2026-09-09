#pragma once
#include <iostream>

namespace InputProcessor::Window {
	struct WindowConfiguration {
		unsigned int Width;
		unsigned int Height;
		std::string Title;
		WindowConfiguration(int width, int height, std::string title) : Width(width), Height(height), Title(title) {}
	};
	
	class IWindow {
	public:
		virtual void Init(const WindowConfiguration& config) = 0;
		virtual void PollEvents() = 0;
		virtual void SwapBuffers() = 0;
		virtual bool CheckShouldClose() = 0;
		virtual void Close() = 0;
	};
}