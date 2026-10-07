#pragma once
#include <unordered_map>
#include <mutex>
#include <memory>
#include <Model/IRenderResourceFactory.hpp>

namespace CoreEngine::Renderer {
	class RenderResourceFactory : public IRenderResourceFactory {
	public:
		RenderResourceFactory() = default;
		~RenderResourceFactory() override;
		ITexture* CreateTexture(const std::string& filePath) override;

	private:
		std::unordered_map<std::string, std::unique_ptr<ITexture>> mTextureCache;
		std::mutex mTextureCacheMutex; // bảo vệ cache khi nhiều thread cùng CreateTexture()
	};
}