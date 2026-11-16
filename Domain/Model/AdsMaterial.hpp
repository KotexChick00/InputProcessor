#pragma once
#include <Model/IMaterial.hpp>
#include <glm/glm.hpp>

namespace Model {
	class AdsMaterial : public IMaterial {
	public:
		AdsMaterial(
			std::vector<CoreEngine::ITexture*> ambientTextures,
			std::vector<CoreEngine::ITexture*> diffuseTextures,
			std::vector<CoreEngine::ITexture*> specularTextures,
			glm::vec4 baseColor = glm::vec4(1.0f)
		);

		void Apply(CoreEngine::IShader* shader) override;

	private:
		glm::vec4 mBaseColor;
		std::vector<CoreEngine::ITexture*> mAmbientTextures;
		std::vector<CoreEngine::ITexture*> mDiffuseTextures;
		std::vector<CoreEngine::ITexture*> mSpecularTextures;
	};
}
