#pragma once
#include <vector>
#include <Renderer/IRenderer.hpp>
#include <Renderer/Resource/IRenderable.hpp>
#include <Model/Mesh.hpp>

namespace Model {
	class RenderModel : public CoreEngine::Renderer::IRenderable {
	public:
		explicit RenderModel(std::vector<Mesh> meshes);
		~RenderModel();

		void Render(CoreEngine::Renderer::IRenderer* renderer, CoreEngine::Renderer::IShader* shader);

		void Draw(CoreEngine::Renderer::IRenderer* renderer, CoreEngine::Renderer::IShader* shader) override {Render(renderer, shader);};

	private:
		struct SubMesh {
			unsigned int IndexCount = 0;
			unsigned int StartIndexByteOffset = 0;
			IMaterial* Material = nullptr;
	
		};

		void UploadToGPU(CoreEngine::Renderer::IResourceManager* resourceManager);

	private:
		std::vector<Mesh> mMeshes;
		std::vector<SubMesh> mSubMeshes;
		CoreEngine::Renderer::IVertexBuffer* mVertexBuffer = nullptr;
		CoreEngine::Renderer::IIndexBuffer* mIndexBuffer = nullptr;
	};
}