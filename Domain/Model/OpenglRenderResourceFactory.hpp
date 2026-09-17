#pragma once
#include <Model/IRenderResourceFactory.hpp>

namespace InputProcessor::Domain::Model {
	class OpenglRenderResourceFactory : public IRenderResourceFactory {
	public:
		virtual InputProcessor::Renderer::Resource::IVertexBuffer* CreateVertexBuffer() override;
		virtual InputProcessor::Renderer::Resource::IIndexBuffer* CreateIndexBuffer() override;
	};
}