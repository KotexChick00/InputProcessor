#include <Mesh/BaseMeshRender.hpp>
#include <Resource/ResouceManager.hpp>

namespace InputProcessor::Domain::Model {
	using namespace InputProcessor::Resource;

	BaseMeshRender::BaseMeshRender(MeshData meshData) {
		mVertexBuffer = ResourceManager::CreateVertexBuffer();
		// Position: 0
		mVertexBuffer->SetData(
			0, 
			&meshData.Vertices[0], 
			meshData.Vertices.size() * sizeof(Vertex), 
			3, 
			sizeof(Vertex), 
			offsetof(Vertex, Position)
		);

		mVertexBuffer->SetData(
			1,
			&meshData.Vertices[0],
			meshData.Vertices.size() * sizeof(Vertex),
			3,
			sizeof(Vertex),
			offsetof(Vertex, Normal)
		);

		mVertexBuffer->SetData(
			1,
			&meshData.Vertices[0],
			meshData.Vertices.size() * sizeof(Vertex),
			2,
			sizeof(Vertex),
			offsetof(Vertex, UV)
		);

		mIndicesCount = meshData.Indices.size();
		mIndexBuffer = ResourceManager::CreateIndexBuffer();
		mIndexBuffer->SetData(&meshData.Indices[0], meshData.Indices.size() * sizeof(unsigned int));
	}
	
	void BaseMeshRender::Render(Transform transform) {
		mVertexBuffer->Bind();
		mIndexBuffer->Bind();
		sRendererCommand->DrawIndex(InputProcessor::Renderer::RenderMode::Triangles, mIndicesCount);
	}
}