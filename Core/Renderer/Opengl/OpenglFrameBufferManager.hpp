#pragma once

#include <unordered_map>
#include <memory>
#include <Renderer/Resource/IFrameBuffer.hpp>
#include <Renderer/Resource/IFrameBufferManager.hpp>
#include <Renderer/Opengl/OpenglFrameBuffer.hpp>

namespace CoreEngine::Renderer::Opengl {
	class OpenglFrameBufferManager : public IFrameBufferManager {
	public:
		static OpenglFrameBufferManager* GetInstance();
		IFrameBuffer* GetFrameBuffer(FrameBufferID frameBufferId) override;

		void InsertFrameBuffer(IFrameBuffer* frameBuffer);
		void DeleteFrameBuffer(FrameBufferID frameBufferId);

		static void Free();

	private:
		OpenglFrameBufferManager() = default;
		~OpenglFrameBufferManager();

		std::unordered_map<FrameBufferID, IFrameBuffer*> mBuffers;
		bool isOnFree = false;

		inline static OpenglFrameBufferManager* sInstance = nullptr;
	};
}
