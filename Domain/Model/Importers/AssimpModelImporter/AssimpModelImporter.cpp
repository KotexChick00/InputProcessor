#include "AssimpModelImporter.hpp"
#include <Logger/Logger.hpp>
#include <Model/AdsMaterial.hpp>
#include <filesystem>

namespace Model {
	AssimpModelImporter::AssimpModelImporter(Renderer::IRenderResourceFactory* materialResourceFactory)
		: materialResourceFactory(materialResourceFactory) {}

	AssimpModelImporter::~AssimpModelImporter() {
		for (auto& [path, texture] : materialTextureCache) {
			delete texture;
		}
		materialTextureCache.clear();
	}

	std::unique_ptr<RenderModel> AssimpModelImporter::Import(const char* file) {
		Assimp::Importer importer; // local biến, mỗi thread gọi Import() có importer riêng — vốn đã an toàn

		const aiScene* scene = importer.ReadFile(
			file,
			aiProcess_Triangulate
			| aiProcess_FlipUVs
			| aiProcess_CalcTangentSpace
			| aiProcess_GenNormals
		);
		if (!scene
			|| scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE
			|| !scene->mRootNode
			) {
			IP_ENGINE_ERROR(
				"AssimpModelImporter::Import: Failed to load model '{}': {}",
				file,
				importer.GetErrorString()
			);
			return nullptr;
		}

		IP_ENGINE_TRACE("AssimpModelImporter::Import: Successfully loaded model '{}'", file);

		// local biến thay vì member — mỗi lời gọi Import() có bản riêng, không đụng nhau giữa các thread
		std::string directory = std::filesystem::path(file).parent_path().string();

		std::vector<Mesh> meshes;
		ProcessNode(scene->mRootNode, scene, meshes, aiMatrix4x4(), directory);

		IP_ENGINE_DEBUG("AssimpModelImporter::Import: Processed {} meshes from model '{}'", meshes.size(), file);

		return std::unique_ptr<RenderModel>(std::make_unique<RenderModel>(meshes));
	}

	void AssimpModelImporter::ProcessNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes, const aiMatrix4x4& parentTransform, const std::string& directory) {
		aiMatrix4x4 nodeTransform = parentTransform * node->mTransformation;
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(ProcessMesh(mesh, scene, nodeTransform, directory));
		}
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			ProcessNode(node->mChildren[i], scene, meshes, nodeTransform, directory);
		}
	}

	Mesh AssimpModelImporter::ProcessMesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4& transform, const std::string& directory) {
		Mesh resultMesh;

		aiMatrix3x3 normalMatrix = aiMatrix3x3(transform);
		normalMatrix.Inverse().Transpose();

		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex;

			aiVector3D transformedPosition = transform * mesh->mVertices[i];
			vertex.Position = glm::vec3(
				transformedPosition.x,
				transformedPosition.y, 
				transformedPosition.z);

			aiVector3D transformedNormal = normalMatrix * mesh->mNormals[i];
			transformedNormal.Normalize();
			vertex.Normal = glm::vec3(	
				transformedNormal.x, 
				transformedNormal.y, 
				transformedNormal.z);

			if (mesh->mTextureCoords[0]) {
				vertex.TextureCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			}

			if (mesh->HasVertexColors(0)) {
				vertex.Color = glm::vec3(
					mesh->mColors[0][i].r,
					mesh->mColors[0][i].g,
					mesh->mColors[0][i].b);
			}

			resultMesh.Vertices.push_back(vertex);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				resultMesh.Indices.push_back(face.mIndices[j]);
			}
		}

		resultMesh.Material.reset(ProcessMaterial(scene->mMaterials[mesh->mMaterialIndex], scene, directory));

		return resultMesh;
	}

	IMaterial* AssimpModelImporter::ProcessMaterial(aiMaterial* material, const aiScene* scene, const std::string& directory) {
		auto ambientTextures = LoadMaterialTextures(material, aiTextureType_AMBIENT, directory);
		auto diffuseTextures = LoadMaterialTextures(material, aiTextureType_DIFFUSE, directory);
		auto specularTextures = LoadMaterialTextures(material, aiTextureType_SPECULAR, directory);

		return new AdsMaterial(ambientTextures, diffuseTextures, specularTextures);
	}

	std::vector<Renderer::ITexture*> AssimpModelImporter::LoadMaterialTextures(aiMaterial* material, aiTextureType type, const std::string& directory) {
		std::vector<Renderer::ITexture*> textures;

		for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
			aiString str;
			material->GetTexture(type, i, &str);
			std::string texturePath = directory + "/" + str.C_Str();

			// khóa cache lại vì nhiều thread có thể đọc/ghi materialTextureCache cùng lúc
			std::lock_guard<std::mutex> lock(materialTextureCacheMutex);

			auto it = materialTextureCache.find(texturePath);
			if (it != materialTextureCache.end()) {
				textures.push_back(it->second);
				continue;
			}

			Renderer::ITexture* texture = materialResourceFactory->CreateTexture(texturePath);
			if (texture) {
				materialTextureCache[texturePath] = texture;
				textures.push_back(texture);
			}
			else {
				IP_ENGINE_WARN("AssimpModelImporter::LoadMaterialTextures: Failed to load texture '{}'", texturePath);
			}
		}

		return textures;
	}
}