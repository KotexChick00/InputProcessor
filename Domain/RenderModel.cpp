#include <Model/RenderModel.hpp>

namespace InputProcessor::Domain::Model {
	RenderModel::RenderModel(std::vector<Mesh*> meshes, IRenderResourceFactory* resourceFactory) : mMeshes(meshes) {
		for (auto mesh : mMeshes) {
			IVertexBuffer* newVertexBuffer = resourceFactory->CreateVertexBuffer();
			newVertexBuffer->SetData(0, &mesh->Vertices[0], mesh->Vertices.size() * sizeof(Vertex), 3, sizeof(Vertex), offsetof(Vertex, Position));
			newVertexBuffer->SetData(1, &mesh->Vertices[0], mesh->Vertices.size() * sizeof(Vertex), 3, sizeof(Vertex), offsetof(Vertex, Normal));
			newVertexBuffer->SetData(2, &mesh->Vertices[0], mesh->Vertices.size() * sizeof(Vertex), 3, sizeof(Vertex), offsetof(Vertex, Color));
			newVertexBuffer->SetData(3, &mesh->Vertices[0], mesh->Vertices.size() * sizeof(Vertex), 2, sizeof(Vertex), offsetof(Vertex, TextureCoords));
		
			mVertexBuffers.push_back(newVertexBuffer);

			IIndexBuffer* newIndexBuffer = resourceFactory->CreateIndexBuffer();
			newIndexBuffer->SetData(&mesh->Indices[0], mesh->Indices.size() * sizeof(unsigned int));

			mIndexBuffers.push_back(newIndexBuffer);
		}
	}

	RenderModel::~RenderModel() {
		for (auto vertexBuffer : mVertexBuffers) delete vertexBuffer;
		for (auto indexBuffer : mIndexBuffers) delete indexBuffer;
		for (auto mesh : mMeshes) delete mesh;
	}

	void RenderModel::Render(Renderer::IRenderer* renderer, IShader* shader) {
		for (size_t i = 0;i < mVertexBuffers.size();++i) {
			IVertexBuffer* vertexBuffer = mVertexBuffers[i];
			IIndexBuffer* indexBuffer = mIndexBuffers[i];
			Mesh* mesh = mMeshes[i];
			shader->Use();
			mesh->Material->Setup(shader);
			vertexBuffer->Bind();
			indexBuffer->Bind();
			renderer->GetRendererCommand()->DrawIndex(InputProcessor::Renderer::RenderMode::Triangles, mesh->Indices.size());
		}
	}
}