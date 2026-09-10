#pragma once
#include <iostream>
#include <vector>
#include <IRenderable.hpp>
#include <Mesh/MeshData.hpp>
#include <Resource/IVertexBuffer.hpp>
#include <Resource/IIndexBuffer.hpp>
#include <Renderer/IRendererCommand.hpp>

namespace InputProcessor::Domain::Model {
	class BaseMeshRender : public IRenderable {
	public:
		BaseMeshRender(MeshData meshData);
		void Render(Transform transform) override;

	private:
		InputProcessor::Resource::IVertexBuffer* mVertexBuffer = nullptr;
		InputProcessor::Resource::IIndexBuffer* mIndexBuffer = nullptr;

		unsigned int mIndicesCount = 0;

	private:
		inline static std::shared_ptr<InputProcessor::Renderer::IRendererCommand> sRendererCommand = nullptr;

	public:
		static void SetRendererCommand(std::shared_ptr<InputProcessor::Renderer::IRendererCommand> rendererCommand) { sRendererCommand = rendererCommand; }
	};
}