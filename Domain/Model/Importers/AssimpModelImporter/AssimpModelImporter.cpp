#include "AssimpModelImporter.hpp"
#include <Logger/Logger.hpp>
#include <Model/AdsMaterial.hpp>
#include <filesystem>
#include <stdexcept>
#include <algorithm>
#include <cmath>

namespace Model {
	AssimpModelImporter::AssimpModelImporter(CoreEngine::IResourceManager* resourceManager)
		: mResourceManager(resourceManager) {}

	namespace {
		glm::mat4 ToGlm(const aiMatrix4x4& m) {
			// Assimp lưu row-major, GLM column-major.
			return glm::mat4(
				m.a1, m.b1, m.c1, m.d1,
				m.a2, m.b2, m.c2, m.d2,
				m.a3, m.b3, m.c3, m.d3,
				m.a4, m.b4, m.c4, m.d4);
		}

		size_t CountNodes(const aiNode* node) {
			size_t count = 1;
			for (unsigned int i = 0; i < node->mNumChildren; i++) count += CountNodes(node->mChildren[i]);
			return count;
		}

		// Đổi aiLight sang ImportedLight, áp `transform` lên vị trí/hướng. false nếu loại đèn không hỗ trợ.
		bool ConvertLight(const aiLight* light, const aiMatrix4x4& transform, ImportedLight& result) {
			result.Name = light->mName.C_Str();
			switch (light->mType) {
			case aiLightSource_DIRECTIONAL: result.Type = LightType::Directional; break;
			case aiLightSource_POINT:       result.Type = LightType::Point; break;
			case aiLightSource_SPOT:        result.Type = LightType::Spot; break;
			default:
				IP_ENGINE_WARN("AssimpModelImporter: Unsupported light type {} for '{}', skip", static_cast<int>(light->mType), result.Name);
				return false;
			}

			aiVector3D position = transform * light->mPosition;
			aiVector3D direction = aiMatrix3x3(transform) * light->mDirection;
			if (direction.SquareLength() > 0.0f) direction.Normalize();

			result.Position = glm::vec3(position.x, position.y, position.z);
			result.Direction = glm::vec3(direction.x, direction.y, direction.z);
			result.Color = glm::vec3(light->mColorDiffuse.r, light->mColorDiffuse.g, light->mColorDiffuse.b);
			result.InnerConeAngle = light->mAngleInnerCone;
			result.OuterConeAngle = light->mAngleOuterCone;
			return true;
		}
	}

	const aiScene* AssimpModelImporter::ReadScene(Assimp::Importer& importer, const char* file) {
		mEmbeddedTextures.clear(); // Assimp texture pointers belong to this import's scene.
		if (!file || !*file) return nullptr;

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
				"AssimpModelImporter: Failed to load model '{}': {}",
				file,
				importer.GetErrorString()
			);
			return nullptr;
		}
		return scene;
	}

	std::unique_ptr<RenderModel> AssimpModelImporter::Import(const char* file) {
		Assimp::Importer importer;
		const aiScene* scene = ReadScene(importer, file);
		if (!scene) return nullptr;

		IP_ENGINE_TRACE("AssimpModelImporter::Import: Successfully loaded model '{}'", file);

		std::string directory = std::filesystem::path(file).parent_path().string();

		std::vector<Mesh> meshes;
		ProcessNode(scene->mRootNode, scene, meshes, aiMatrix4x4(), directory);

		IP_ENGINE_DEBUG("AssimpModelImporter::Import: Processed {} meshes from model '{}'", meshes.size(), file);

		auto model = std::make_unique<RenderModel>(std::move(meshes));
		model->SetLights(ProcessLights(scene));
		IP_ENGINE_DEBUG("AssimpModelImporter::Import: Found {} lights in model '{}'", model->GetLights().size(), file);
		return model;
	}

	std::vector<ImportedLight> AssimpModelImporter::ProcessLights(const aiScene* scene) {
		std::vector<ImportedLight> lights;
		for (unsigned int i = 0; i < scene->mNumLights; i++) {
			const aiLight* light = scene->mLights[i];
			// Vị trí/hướng của aiLight nằm trong hệ toạ độ của node cùng tên -> cộng dồn transform lên tới gốc.
			aiMatrix4x4 world;
			for (const aiNode* node = scene->mRootNode->FindNode(light->mName); node; node = node->mParent) {
				world = node->mTransformation * world;
			}
			ImportedLight result;
			if (ConvertLight(light, world, result)) lights.push_back(std::move(result));
		}
		return lights;
	}

	std::unique_ptr<ImportedScene> AssimpModelImporter::ImportScene(const char* file, size_t maxNodes) {
		Assimp::Importer importer;
		const aiScene* scene = ReadScene(importer, file);
		if (!scene) return nullptr;

		std::string directory = std::filesystem::path(file).parent_path().string();
		auto result = std::make_unique<ImportedScene>();

		const size_t nodeCount = CountNodes(scene->mRootNode);
		if (nodeCount > maxNodes) {
			// Quá nhiều node cho ECS: gộp như bản A (đỉnh và đèn theo toạ độ thế giới).
			IP_ENGINE_WARN("AssimpModelImporter::ImportScene: '{}' has {} nodes (limit {}), flattening into one node", file, nodeCount, maxNodes);
			std::vector<Mesh> meshes;
			ProcessNode(scene->mRootNode, scene, meshes, aiMatrix4x4(), directory);
			ImportedNode node;
			node.Name = scene->mRootNode->mName.C_Str();
			if (!meshes.empty()) node.Model = std::make_unique<RenderModel>(std::move(meshes));
			node.Lights = ProcessLights(scene);
			result->Nodes.push_back(std::move(node));
			result->Flattened = true;
			return result;
		}

		result->Nodes.reserve(nodeCount);
		ProcessSceneNode(scene->mRootNode, -1, scene, directory, *result);
		IP_ENGINE_DEBUG("AssimpModelImporter::ImportScene: Read {} nodes from '{}'", result->Nodes.size(), file);
		return result;
	}

	void AssimpModelImporter::ProcessSceneNode(const aiNode* node, int parent, const aiScene* scene, const std::string& directory, ImportedScene& result) {
		ImportedNode imported;
		imported.Name = node->mName.C_Str();
		imported.Parent = parent;
		imported.Local = ToGlm(node->mTransformation);

		// Tách TRS; nếu ghép lại không khớp (có shear) thì dùng thẳng ma trận.
		aiVector3D scaling, position;
		aiQuaternion rotation;
		node->mTransformation.Decompose(scaling, rotation, position);
		imported.Position = glm::vec3(position.x, position.y, position.z);
		imported.Rotation = glm::quat(rotation.w, rotation.x, rotation.y, rotation.z);
		imported.Scale = glm::vec3(scaling.x, scaling.y, scaling.z);
		const aiMatrix4x4 recomposed(scaling, rotation, position);
		for (unsigned int row = 0; row < 4 && imported.IsTrs; row++) {
			for (unsigned int column = 0; column < 4; column++) {
				const float expected = node->mTransformation[row][column];
				if (std::fabs(recomposed[row][column] - expected) > 1e-4f * std::max(1.0f, std::fabs(expected))) {
					imported.IsTrs = false;
					break;
				}
			}
		}

		std::vector<Mesh> meshes;
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			meshes.push_back(ProcessMesh(scene->mMeshes[node->mMeshes[i]], scene, aiMatrix4x4(), directory));
		}
		if (!meshes.empty()) imported.Model = std::make_unique<RenderModel>(std::move(meshes));

		// Đèn thuộc node đầu tiên trùng tên (giống cách Assimp và bản A tra cứu).
		for (unsigned int i = 0; i < scene->mNumLights; i++) {
			const aiLight* light = scene->mLights[i];
			if (scene->mRootNode->FindNode(light->mName) != node) continue;
			ImportedLight converted;
			if (ConvertLight(light, aiMatrix4x4(), converted)) imported.Lights.push_back(std::move(converted));
		}

		const int index = static_cast<int>(result.Nodes.size());
		result.Nodes.push_back(std::move(imported));
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			ProcessSceneNode(node->mChildren[i], index, scene, directory, result);
		}
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

	std::vector<CoreEngine::ITexture*> AssimpModelImporter::LoadMaterialTextures(aiMaterial* material, aiTextureType type, const aiScene* scene, const std::string& directory) {
		std::vector<CoreEngine::ITexture*> textures;

		for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
			aiString str;
			material->GetTexture(type, i, &str);

			if (!mResourceManager) throw std::runtime_error("A resource manager is required to load model textures");
			std::string texturePath = (std::filesystem::path(directory) / str.C_Str()).string();
			CoreEngine::ITexture* texture = nullptr;
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
