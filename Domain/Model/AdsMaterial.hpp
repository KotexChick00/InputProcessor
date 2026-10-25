#pragma once
#include <Model/IMaterial.hpp>
#include <glm/glm.hpp>

namespace Model {
	class AdsMaterial : public IMaterial {
	public:
		AdsMaterial(
			std::vector<CoreEngine::Renderer::ITexture*> ambientTextures,
			std::vector<CoreEngine::Renderer::ITexture*> diffuseTextures,
			std::vector<CoreEngine::Renderer::ITexture*> specularTextures,
			glm::vec4 baseColor = glm::vec4(1.0f)
		);

		void Apply(CoreEngine::Renderer::IShader* shader) override;

	private:
		glm::vec4 mBaseColor;
		std::vector<CoreEngine::Renderer::ITexture*> mAmbientTextures;
		std::vector<CoreEngine::Renderer::ITexture*> mDiffuseTextures;
		std::vector<CoreEngine::Renderer::ITexture*> mSpecularTextures;
	};
}
