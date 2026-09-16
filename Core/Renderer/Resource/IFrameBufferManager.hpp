#pragma once
#include <iostream>
#include <Renderer/Resource/IFrameBuffer.hpp>

namespace InputProcessor::Renderer::Resource {
	class IFrameBufferManager {
	public:
		virtual ~IFrameBufferManager() = default;
		virtual IFrameBuffer* GetFrameBuffer(FrameBufferID frameBufferId) = 0;
	};
}