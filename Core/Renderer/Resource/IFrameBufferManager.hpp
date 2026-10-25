#pragma once
#include <pch.h>
#include <Renderer/Resource/IFrameBuffer.hpp>

namespace CoreEngine::Renderer {
	class CORE_API IFrameBufferManager {
	public:
		virtual ~IFrameBufferManager() = default;
		virtual IFrameBuffer* GetFrameBuffer(FrameBufferID frameBufferId) = 0;
	};
}