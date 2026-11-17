#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Model/Importers/IModelImporter.hpp>
#include <Model/ImportedScene.hpp>
#include <Renderer/Resource/IResourceManager.hpp>

namespace Model {


	class AssimpModelImporter : public IModelImporter, public ISceneImporter {
	public:
		explicit AssimpModelImporter(CoreEngine::IResourceManager* resourceManager);
		// Bản A: gộp mọi mesh thành 1 RenderModel, đỉnh đã áp node transform.
		std::unique_ptr<RenderModel> Import(const char* file) override;
		// Bản B: giữ cây node, mỗi node có mesh/đèn theo toạ độ cục bộ.
		std::unique_ptr<ImportedScene> ImportScene(const char* file, size_t maxNodes) override;
		~AssimpModelImporter() override = default;

	private:
		void ProcessNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& outMeshes, const aiMatrix4x4& parentTransform, const std::string& directory);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4& transform, const std::string& directory);
		IMaterial* ProcessMaterial(aiMaterial* material, const aiScene* scene, const std::string& directory);
		std::vector<ImportedLight> ProcessLights(const aiScene* scene);
		const aiScene* ReadScene(Assimp::Importer& importer, const char* file);
		void ProcessSceneNode(const aiNode* node, int parent, const aiScene* scene, const std::string& directory, ImportedScene& result);
		std::vector<CoreEngine::ITexture*> LoadMaterialTextures(aiMaterial* material, aiTextureType type, const aiScene* scene, const std::string& directory);

	private:
		CoreEngine::IResourceManager* mResourceManager;
		std::unordered_map<const aiTexture*, CoreEngine::ITexture*> mEmbeddedTextures;
	};
}
