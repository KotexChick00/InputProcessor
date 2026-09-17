#include <Model/OpenglRenderResourceFactory.hpp>
#include <Renderer/Opengl/OpenglVertexBuffer.hpp>
#include <Renderer/Opengl/OpenglIndexBuffer.hpp>

namespace InputProcessor::Domain::Model {
	InputProcessor::Renderer::Resource::IVertexBuffer* OpenglRenderResourceFactory::CreateVertexBuffer() {
		return Renderer::Resource::Opengl::OpenglVertexBuffer::Create();
	}

	InputProcessor::Renderer::Resource::IIndexBuffer* OpenglRenderResourceFactory::CreateIndexBuffer() {
		return Renderer::Resource::Opengl::OpenglIndexBuffer::Create();
	}
}