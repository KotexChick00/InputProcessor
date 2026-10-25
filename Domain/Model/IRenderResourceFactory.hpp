#pragma once
#include <string>
#include <Renderer/Resource/ITexture.hpp>

namespace CoreEngine::Renderer {
	class IRenderResourceFactory {
	public:
		virtual ~IRenderResourceFactory() = default;
		virtual ITexture* CreateTexture(const std::string& filePath) = 0;
	};
}