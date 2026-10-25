#include "AssimpModelImporter.hpp"
#include <Logger/Logger.hpp>
#include <Model/AdsMaterial.hpp>
#include <filesystem>
#include <stdexcept>

namespace Model {
	AssimpModelImporter::AssimpModelImporter(Renderer::IResourceManager* resourceManager)
		: mResourceManager(resourceManager) {}

	std::unique_ptr<RenderModel> AssimpModelImporter::Import(const char* file) {
		mEmbeddedTextures.clear(); // Assimp texture pointers belong to this import's scene.
		if (!file || !*file) return nullptr;
		Assimp::Importer importer;

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

		std::string directory = std::filesystem::path(file).parent_path().string();

		std::vector<Mesh> meshes;
		ProcessNode(scene->mRootNode, scene, meshes, aiMatrix4x4(), directory);

		IP_ENGINE_DEBUG("AssimpModelImporter::Import: Processed {} meshes from model '{}'", meshes.size(), file);

		return std::make_unique<RenderModel>(std::move(meshes));
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
				transformedPosition.z
			);

			if (mesh->HasNormals()) {
				aiVector3D normal = normalMatrix * mesh->mNormals[i];
				normal.Normalize();
				vertex.Normal = glm::vec3(normal.x, normal.y, normal.z);
			}

			if (mesh->mTextureCoords[0]) {
				vertex.TextureCoords = glm::vec2(
					mesh->mTextureCoords[0][i].x, 
					mesh->mTextureCoords[0][i].y
				);
			}

			if (mesh->HasVertexColors(0)) {
				vertex.Color = glm::vec3(
					mesh->mColors[0][i].r, 
					mesh->mColors[0][i].g, 
					mesh->mColors[0][i].b
				);
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
		auto ambientTextures = LoadMaterialTextures(material, aiTextureType_AMBIENT, scene, directory);
		auto diffuseTextures = LoadMaterialTextures(material, aiTextureType_DIFFUSE, scene, directory);
		if (diffuseTextures.empty()) diffuseTextures = LoadMaterialTextures(material, aiTextureType_BASE_COLOR, scene, directory);
		auto specularTextures = LoadMaterialTextures(material, aiTextureType_SPECULAR, scene, directory);

		aiColor4D color(1, 1, 1, 1);
		if (material->Get(AI_MATKEY_BASE_COLOR, color) != AI_SUCCESS)
			material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		return new AdsMaterial(ambientTextures, diffuseTextures, specularTextures, glm::vec4(color.r, color.g, color.b, color.a));
	}

	std::vector<Renderer::ITexture*> AssimpModelImporter::LoadMaterialTextures(aiMaterial* material, aiTextureType type, const aiScene* scene, const std::string& directory) {
		std::vector<Renderer::ITexture*> textures;

		for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
			aiString str;
			material->GetTexture(type, i, &str);

			if (!mResourceManager) throw std::runtime_error("A resource manager is required to load model textures");
			std::string texturePath = (std::filesystem::path(directory) / str.C_Str()).string();
			Renderer::ITexture* texture = nullptr;
			if (const aiTexture* embedded = scene->GetEmbeddedTexture(str.C_Str())) {
				if (embedded->mHeight != 0) throw std::runtime_error("Unsupported uncompressed embedded model texture");
				auto [entry, inserted] = mEmbeddedTextures.try_emplace(embedded, nullptr);
				if (inserted) entry->second = mResourceManager->CreateTextureFromMemory(reinterpret_cast<const unsigned char*>(embedded->pcData), embedded->mWidth);
				texture = entry->second;
			} else {
				texture = mResourceManager->CreateTexture(texturePath);
			}
			if (texture) {
				textures.push_back(texture);
			}
			else {
				IP_ENGINE_WARN("AssimpModelImporter::LoadMaterialTextures: Failed to load texture '{}'", texturePath);
			}
		}

		return textures;
	}
}
