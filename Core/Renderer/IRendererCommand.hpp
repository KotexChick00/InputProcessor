#pragma once
#include <iostream>

namespace InputProcessor::Renderer {
	struct ColorRGB {
		float Red;
		float Green;
		float Blue;
		ColorRGB() : Red(0.f), Green(0.f), Blue(0.f) {}
		ColorRGB(float red, float green, float blue): Red(red), Green(green), Blue(blue) {}
	};

	struct ColorRGBA {
		float Red;
		float Green;
		float Blue;
		float Alpha;
		ColorRGBA() : Red(0.f), Green(0.f), Blue(0.f), Alpha(0.f) {}
		ColorRGBA(float red, float green, float blue, float alpha = 1.f) : Red(red), Green(green), Blue(blue), Alpha(alpha) {}
	};

	enum class BufferFlag {
		Color = 1,
		Depth = 1 << 1
	};

	inline BufferFlag operator|(BufferFlag a, BufferFlag b) {
		return static_cast<BufferFlag>(static_cast<int>(a) | static_cast<int>(b));
	}

	inline bool HasFlag(BufferFlag a, BufferFlag b) {
		return static_cast<bool>(static_cast<int>(a) | static_cast<int>(b));
	}

	enum class RenderMode {
		Triangles = 0
	};
	
	class IRendererCommand {
	public:
		virtual ~IRendererCommand() = default;

		virtual void SetClearColor(ColorRGBA color) = 0;
		virtual void ClearBuffers(BufferFlag bufferFlag) = 0;
		virtual void Draw(RenderMode renderMode, unsigned int count, unsigned int start = 0) = 0;
		virtual void DrawIndex(RenderMode renderMode, unsigned int count, unsigned int start = 0) = 0;
	};
}