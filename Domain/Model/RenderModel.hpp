#pragma once
#include <vector>
#include <Renderer/IRenderer.hpp>
#include <Model/Mesh.hpp>

namespace Model {
	class RenderModel {
	public:
		RenderModel(std::vector<Mesh> meshes);
		~RenderModel();

		void Render(CoreEngine::Renderer::IRenderer* renderer);

	private:
		std::vector<Mesh> mMeshes;
		CoreEngine::Renderer::IVertexBuffer* mVertexBuffer = nullptr;
		CoreEngine::Renderer::IIndexBuffer* mIndexBuffer = nullptr;
	};
}