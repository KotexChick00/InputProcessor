#pragma once
#include <Renderer/RendererPrimitives.hpp>

namespace InputProcessor::Renderer {

	class IRendererCommand {
	public:
		virtual void ClearBuffers(ClearBufferMasks clearBufferMasks) = 0;
		virtual void Draw(RenderMode renderMode, unsigned int count, unsigned int start = 0) = 0;
		virtual void DrawIndex(RenderMode renderMode, unsigned int count, unsigned int start = 0) = 0;
	};
}