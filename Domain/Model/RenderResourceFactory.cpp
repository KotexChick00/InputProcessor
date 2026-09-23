#include <Model/RenderResourceFactory.hpp>
#include <Logger/Logger.hpp>

namespace CoreEngine::Renderer {
	RenderResourceFactory::~RenderResourceFactory() = default;

	ITexture* RenderResourceFactory::CreateTexture(const std::string& filePath) {
		std::lock_guard<std::mutex> lock(mTextureCacheMutex);
		auto it = mTextureCache.find(filePath);
		if (it != mTextureCache.end()) {
			IP_ENGINE_DEBUG("RenderResourceFactory::CreateTexture: Returning cached texture for '{}'", filePath);
			return it->second.get();
		}
		
		auto texture = std::make_unique<ITexture>();
		if (!texture) {
			IP_ENGINE_ERROR("RenderResourceFactory::CreateTexture: Failed to create texture for '{}'", filePath);
			return nullptr;
		}

		ITexture* texturePtr = texture.get();
		mTextureCache[filePath] = std::move(texture);
		IP_ENGINE_DEBUG("RenderResourceFactory::CreateTexture: Created new texture for '{}'", filePath);
		return texturePtr;
	}
}