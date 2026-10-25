#include <Renderer/RendererFactory.hpp>
#include <Renderer/Opengl/OpenglRenderer.hpp>

namespace CoreEngine::Renderer {
	IRenderer* RendererFactory::Create(RendererSpec spec) {
		switch (spec) {
		case RendererSpec::Opengl: return Opengl::OpenglRenderer::GetInstance();
		case RendererSpec::DirectX: 
			CORE_ASSERT(false && "DirectX renderer currently not supported");
			break;
		case RendererSpec::Vulkan: 
			CORE_ASSERT(false && "Vulkan renderer currently not supported");
			break;
		default: 
			CORE_ASSERT(false && "Can't find any spec match");
			break;
		}
		return nullptr;
	}
}