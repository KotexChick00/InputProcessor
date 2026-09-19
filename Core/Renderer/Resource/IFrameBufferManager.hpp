#pragma once
#include <iostream>
#include <Renderer/Resource/IFrameBuffer.hpp>

namespace CoreEngine::Renderer {
	class IFrameBufferManager {
	public:
		virtual ~IFrameBufferManager() = default;
		virtual IFrameBuffer* GetFrameBuffer(FrameBufferID frameBufferId) = 0;
	};
}