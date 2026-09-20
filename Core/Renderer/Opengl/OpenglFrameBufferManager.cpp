#include <Renderer/Opengl/OpenglFrameBufferManager.hpp>
#include <Renderer/Opengl/OpenglFrameBuffer.hpp>
#include <Logger/Logger.hpp>

namespace CoreEngine::Renderer::Opengl {
	using namespace CoreEngine::Logger;

	IFrameBuffer* OpenglFrameBufferManager::GetFrameBuffer(FrameBufferID id) {
		auto it = mBuffers.find(id);
		if (it == mBuffers.end()) return nullptr;
		return it->second;
	}

	OpenglFrameBufferManager::~OpenglFrameBufferManager() {
		isOnFree = true;

		for (auto pair : mBuffers) {
			delete pair.second;
		}
	}

	OpenglFrameBufferManager* OpenglFrameBufferManager::GetInstance() {
		if (sInstance == nullptr) {
			sInstance = new OpenglFrameBufferManager();
		}

		return sInstance;
	}

	void OpenglFrameBufferManager::InsertFrameBuffer(IFrameBuffer* frameBuffer) {
		if (mBuffers.contains(frameBuffer->GetFrameBufferId())) {
			IP_ENGINE_WARN("Framebuffer id {} existed, do nothing", frameBuffer->GetFrameBufferId());
			return;
		}

		mBuffers.insert({ frameBuffer->GetFrameBufferId(), frameBuffer });
	}
	
	void OpenglFrameBufferManager::DeleteFrameBuffer(FrameBufferID frameBufferId) {
		if (isOnFree) return;

		if (mBuffers.contains(frameBufferId)) {
			mBuffers.erase(frameBufferId);
		}
	}

	void OpenglFrameBufferManager::Free() {
		if (sInstance != nullptr) {
			delete sInstance;
			sInstance = nullptr;
		}
	}
}
