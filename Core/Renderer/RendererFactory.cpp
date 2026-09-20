#include <Renderer/RendererFactory.hpp>
#include <Renderer/Opengl/OpenglRenderer.hpp>

namespace CoreEngine::Renderer {
	IRenderer* RendererFactory::Create(RendererSpec spec) {
		switch (spec) {
		case RendererSpec::Opengl: return Opengl::OpenglRenderer::GetInstance();
		case RendererSpec::DirectX: CORE_ASSERT("DirectX renderer currently not supported");
		case RendererSpec::Vulkan: CORE_ASSERT("Vulkan renderer currently not supported");
		default: CORE_ASSERT("Can't find any spec match");
		}
	}
}