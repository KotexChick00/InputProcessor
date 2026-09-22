#pragma once
#include <Model/IMaterial.hpp>

namespace Model {
	class AdsMaterial : public IMaterial {
	public:
		AdsMaterial(
			std::vector<CoreEngine::Renderer::ITexture*> ambientTextures,
			std::vector<CoreEngine::Renderer::ITexture*> mDiffuseTextures,
			std::vector<CoreEngine::Renderer::ITexture*> mSpecularTextures
		);

		void Apply(CoreEngine::Renderer::IShader* shader) override;

	private:
		std::vector<CoreEngine::Renderer::ITexture*> mAmbientTextures;
		std::vector<CoreEngine::Renderer::ITexture*> mDiffuseTextures;
		std::vector<CoreEngine::Renderer::ITexture*> mSpecularTextures;
	};
}