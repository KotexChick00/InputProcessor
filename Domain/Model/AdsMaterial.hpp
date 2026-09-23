#pragma once
#include <Model/IMaterial.hpp>

namespace Model {
	class AdsMaterial : public IMaterial {
	public:
		AdsMaterial(
			std::vector<CoreEngine::Renderer::ITexture*> ambientTextures,
			std::vector<CoreEngine::Renderer::ITexture*> diffuseTextures,
			std::vector<CoreEngine::Renderer::ITexture*> specularTextures
		);

		void Apply(CoreEngine::Renderer::IShader* shader) override;

	private:
		std::vector<CoreEngine::Renderer::ITexture*> mAmbientTextures;
		std::vector<CoreEngine::Renderer::ITexture*> mDiffuseTextures;
		std::vector<CoreEngine::Renderer::ITexture*> mSpecularTextures;
	};
}