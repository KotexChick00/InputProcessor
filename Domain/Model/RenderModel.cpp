#include <Model/RenderModel.hpp>
#include <Logger/Logger.hpp>

namespace Model {
	namespace Renderer = CoreEngine::Renderer;

	RenderModel::RenderModel(std::vector<Mesh> meshes)
		: mMeshes(std::move(meshes)) {}

	RenderModel::~RenderModel() {
		// mVertexBuffer/mIndexBuffer do RenderModel tự tạo riêng cho model này (không share với model khác),
		// nên RenderModel chịu trách nhiệm huỷ — khác với texture (do OpenglResourceManager sở hữu chung)
		delete mVertexBuffer;
		delete mIndexBuffer;
	}

	void RenderModel::UploadToGPU(Renderer::IResourceManager* resourceManager) {
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texCoords;
		std::vector<glm::vec3> colors;
		std::vector<unsigned int> indices;

		unsigned int vertexOffset = 0;
		mSubMeshes.reserve(mMeshes.size());

		for (auto& mesh : mMeshes) {
			for (auto& v : mesh.Vertices) {
				positions.push_back(v.Position);
				normals.push_back(v.Normal);
				texCoords.push_back(v.TextureCoords);
				colors.push_back(v.Color);
			}

			unsigned int startIndex = static_cast<unsigned int>(indices.size());
			for (unsigned int idx : mesh.Indices) {
				// cộng dồn vertexOffset vì index gốc của Assimp chỉ đúng trong phạm vi 1 mesh,
				// khi gộp vào mảng vertex chung phải dịch lại cho khớp vị trí thật
				indices.push_back(idx + vertexOffset);
			}

			SubMesh sub;
			sub.IndexCount = static_cast<unsigned int>(mesh.Indices.size());
			sub.StartIndexByteOffset = startIndex * sizeof(unsigned int);
			sub.Material = mesh.Material.get(); // mượn, không sở hữu
			mSubMeshes.push_back(sub);

			vertexOffset += static_cast<unsigned int>(mesh.Vertices.size());
		}

		mVertexBuffer = resourceManager->CreateVertexBuffer();
		mVertexBuffer->SetData(0, positions.data(), static_cast<unsigned int>(positions.size() * sizeof(glm::vec3)), 3, sizeof(glm::vec3));
		mVertexBuffer->SetData(1, normals.data(), static_cast<unsigned int>(normals.size() * sizeof(glm::vec3)), 3, sizeof(glm::vec3));
		mVertexBuffer->SetData(2, texCoords.data(), static_cast<unsigned int>(texCoords.size() * sizeof(glm::vec2)), 2, sizeof(glm::vec2));
		mVertexBuffer->SetData(3, colors.data(), static_cast<unsigned int>(colors.size() * sizeof(glm::vec3)), 3, sizeof(glm::vec3));

		mIndexBuffer = resourceManager->CreateIndexBuffer();
		mIndexBuffer->SetData(indices.data(), static_cast<unsigned int>(indices.size() * sizeof(unsigned int)));
	}

	void RenderModel::Render(Renderer::IRenderer* renderer, Renderer::IShader* shader) {
		if (!renderer || !shader) {
			IP_ENGINE_WARN("RenderModel::Render: renderer or shader is null, skip");
			return;
		}

		// upload GPU 1 lần duy nhất, trễ tới khi thực sự cần render (lazy init)
		if (mVertexBuffer == nullptr || mIndexBuffer == nullptr) {
			UploadToGPU(renderer->GetResourceManager());
		}

		mVertexBuffer->Bind();
		mIndexBuffer->Bind();

		for (auto& sub : mSubMeshes) {
			if (sub.Material) {
				sub.Material->Apply(shader);
			}
			renderer->GetRendererCommand()->DrawIndex(
				Renderer::RenderMode::Triangles,
				sub.IndexCount,
				sub.StartIndexByteOffset
			);
		}

		mIndexBuffer->UnBind();
		mVertexBuffer->UnBind();
	}
}