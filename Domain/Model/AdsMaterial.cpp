#include <Model/AdsMaterial.hpp>
#include <memory>

namespace Model {
	AdsMaterial::AdsMaterial(
		std::vector<CoreEngine::Renderer::ITexture*> ambientTextures,
		std::vector<CoreEngine::Renderer::ITexture*> diffuseTextures,
		std::vector<CoreEngine::Renderer::ITexture*> specularTextures,
		glm::vec4 baseColor
	) : mBaseColor(baseColor), mAmbientTextures(ambientTextures), mDiffuseTextures(diffuseTextures), mSpecularTextures(specularTextures) {  }

	void AdsMaterial::Apply(CoreEngine::Renderer::IShader* shader) {
		shader->SetUniform1i("uHasDiffuseTexture", !mDiffuseTextures.empty());
		shader->SetUniform4f("uBaseColor", mBaseColor.r, mBaseColor.g, mBaseColor.b, mBaseColor.a);
		for (size_t i = 0;i < mAmbientTextures.size(); ++i) {
			std::string textureName = "material.ambient[" + std::to_string(i) + "]";
			shader->SetUniformTexture(textureName, mAmbientTextures[i]);
		}

		for (size_t i = 0;i < mDiffuseTextures.size(); ++i) {
			std::string textureName = "material.diffuse[" + std::to_string(i) + "]";
			shader->SetUniformTexture(textureName, mDiffuseTextures[i]);
		}

		for (size_t i = 0;i < mSpecularTextures.size(); ++i) {
			std::string textureName = "material.specular[" + std::to_string(i) + "]";
			shader->SetUniformTexture(textureName, mSpecularTextures[i]);
		}
	}
}
