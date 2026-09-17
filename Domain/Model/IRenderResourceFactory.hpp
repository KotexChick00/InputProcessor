#pragma once
#include <iostream>
#include <Renderer/Resource/IVertexBuffer.hpp>
#include <Renderer/Resource/IIndexBuffer.hpp>

namespace InputProcessor::Domain::Model {
	class IRenderResourceFactory {
	public:
		virtual InputProcessor::Renderer::Resource::IVertexBuffer* CreateVertexBuffer() = 0;
		virtual InputProcessor::Renderer::Resource::IIndexBuffer* CreateIndexBuffer() = 0;
	};
}