#pragma once
#include <Model/Mesh.hpp>
#include <Renderer/IRenderer.hpp>
#include <Renderer/Resource/IVertexBuffer.hpp>
#include <Renderer/Resource/IIndexBuffer.hpp>
#include <Model/IRenderResourceFactory.hpp>
#include <vector>

namespace InputProcessor::Domain::Model {
	class RenderModel {
	public:
		RenderModel(std::vector<Mesh*> meshes, IRenderResourceFactory* resourceFactory);
		~RenderModel();

		void Render(Renderer::IRenderer* renderer, IShader* shader);

	private:
		std::vector<Mesh*> mMeshes;
		std::vector<IVertexBuffer*> mVertexBuffers;
		std::vector<IIndexBuffer*> mIndexBuffers;
	};
};