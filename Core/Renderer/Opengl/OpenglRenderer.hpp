#pragma once
#include <Renderer/IRenderer.hpp>
#include <Renderer/Opengl/OpenglRendererCommand.hpp>
#include <Renderer/Opengl/OpenglResourceManager.hpp>
#include <Renderer/Opengl/OpenglFrameBufferManager.hpp>
#include <memory>

namespace CoreEngine::Renderer::Opengl {
	class OpenglRenderer : public IRenderer {
	public:
		~OpenglRenderer() override;
		static OpenglRenderer* GetInstance();
		
		static void SFree();

		void Free() override;
		void Config(RendererConfiguration configuration) override;
		IRendererCommand* GetRendererCommand() override;
		IResourceManager* GetResourceManager() override;
		IFrameBufferManager* GetFrameBufferManager() override;

		RendererConfiguration GetConfig() override;

	private:
		OpenglRenderer();

		RendererConfiguration mConfiguration;

		inline static OpenglRenderer* sInstance = nullptr;
	};
}
