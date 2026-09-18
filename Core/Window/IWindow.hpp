#pragma once
#include <iostream>

namespace InputProcessor::Window {
	struct WindowConfiguration {
		unsigned int Width;
		unsigned int Height;
		std::string Title;
		unsigned int TargetFPS;
		WindowConfiguration(int width, int height, std::string title, unsigned int targetFPS = 0)
			: Width(width), Height(height), Title(title), TargetFPS(targetFPS) {}
	};

	class IWindowVisitor;
	
	class IWindow {
	public:
		virtual void Init(const WindowConfiguration& config) = 0;
		virtual void PollEvents() = 0;
		virtual void SwapBuffers() = 0;
		virtual bool CheckShouldClose() = 0;
		virtual void Close() = 0;

		virtual void Accept(IWindowVisitor* visitor) = 0;
	};
}