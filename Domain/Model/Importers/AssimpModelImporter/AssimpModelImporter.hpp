#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Model/Importers/IModelImporter.hpp>
#include <Renderer/Resource/IResouceManager.hpp>

namespace Model {

	namespace Renderer = CoreEngine::Renderer;

	class AssimpModelImporter : public IModelImporter {
	public:
		explicit AssimpModelImporter(Renderer::IResourceManager* resourceManager);
		std::unique_ptr<RenderModel> Import(const char* file) override;
		~AssimpModelImporter() override = default;

	private:
		void ProcessNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& outMeshes, const aiMatrix4x4& parentTransform, const std::string& directory);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4& transform, const std::string& directory);
		IMaterial* ProcessMaterial(aiMaterial* material, const aiScene* scene, const std::string& directory);
		std::vector<Renderer::ITexture*> LoadMaterialTextures(aiMaterial* material, aiTextureType type, const std::string& directory);

	private:
		Renderer::IResourceManager* mResourceManager;
	};
}