#pragma once
#include <vector>
#include <Renderer/IRenderer.hpp>
#include <Model/Mesh.hpp>

namespace Model {
	class RenderModel {
	public:
		explicit RenderModel(std::vector<Mesh> meshes);
		~RenderModel();
		const std::vector<Mesh>& GetMeshes() const { return mMeshes; }

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
		CoreEngine::IVertexBuffer* mVertexBuffer = nullptr;
		CoreEngine::IIndexBuffer* mIndexBuffer = nullptr;
	};
}
