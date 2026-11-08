#pragma once
#include <unordered_map>
#include <mutex>
#include <memory>
#include <Renderer/Resource/IResourceManager.hpp>
#include <Model/IRenderResourceFactory.hpp>

namespace CoreEngine::Renderer {
        // Wraps an IResourceManager with a path-keyed texture cache.
        // Returned ITexture* pointers are non-owning; the manager (and this
        // factory's cache) retains ownership.
        class RenderResourceFactory : public IRenderResourceFactory {
        public:
                explicit RenderResourceFactory(IResourceManager* resourceManager)
                        : mResourceManager(resourceManager) {}

                ~RenderResourceFactory() override = default;

                ITexture* CreateTexture(const std::string& filePath) override;

        private:
                IResourceManager* mResourceManager = nullptr;
                std::unordered_map<std::string, ITexture*> mTextureCache;
                std::mutex mTextureCacheMutex; // bảo vệ cache khi nhiều thread cùng CreateTexture()
        };
}
