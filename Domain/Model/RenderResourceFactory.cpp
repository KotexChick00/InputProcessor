#include <Model/RenderResourceFactory.hpp>
#include <Logger/Logger.hpp>

namespace CoreEngine::Renderer {

        ITexture* RenderResourceFactory::CreateTexture(const std::string& filePath) {
                if (!mResourceManager) {
                        IP_ENGINE_ERROR("RenderResourceFactory::CreateTexture: no IResourceManager attached");
                        return nullptr;
                }
                std::lock_guard<std::mutex> lock(mTextureCacheMutex);
                auto it = mTextureCache.find(filePath);
                if (it != mTextureCache.end()) {
                        IP_ENGINE_DEBUG("RenderResourceFactory::CreateTexture: Returning cached texture for '{}'", filePath);
                        return it->second;
                }

                // The IResourceManager owns the GPU texture; we cache the
                // returned non-owning pointer so repeated requests for the same
                // path skip the disk read + GL upload.
                ITexture* texture = mResourceManager->CreateTexture(filePath);
                if (!texture) {
                        IP_ENGINE_ERROR("RenderResourceFactory::CreateTexture: Failed to load texture '{}'", filePath);
                        return nullptr;
                }
                mTextureCache[filePath] = texture;
                IP_ENGINE_DEBUG("RenderResourceFactory::CreateTexture: Created new texture for '{}'", filePath);
                return texture;
        }

}
