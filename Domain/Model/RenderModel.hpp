#pragma once
#include <vector>
#include <Renderer/IRenderer.hpp>
#include <Model/Mesh.hpp>
#include <Model/ImportedLight.hpp>

namespace Model {
	class RenderModel {
	public:
		explicit RenderModel(std::vector<Mesh> meshes);
		~RenderModel();
		const std::vector<Mesh>& GetMeshes() const { return mMeshes; }

		// Nguồn sáng có trong file (rỗng nếu file không có).
		const std::vector<ImportedLight>& GetLights() const { return mLights; }
		void SetLights(std::vector<ImportedLight> lights) { mLights = std::move(lights); }

		void Render(CoreEngine::IRenderer* renderer, CoreEngine::IShader* shader);

	private:
		struct SubMesh {
			unsigned int IndexCount = 0;
			unsigned int StartIndexByteOffset = 0;
			IMaterial* Material = nullptr;
	
		};

		void UploadToGPU(CoreEngine::IResourceManager* resourceManager);

	private:
		std::vector<Mesh> mMeshes;
		std::vector<SubMesh> mSubMeshes;
		std::vector<ImportedLight> mLights;
		CoreEngine::IVertexBuffer* mVertexBuffer = nullptr;
		CoreEngine::IIndexBuffer* mIndexBuffer = nullptr;
	};
}
