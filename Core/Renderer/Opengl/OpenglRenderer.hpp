#pragma once
#include <Renderer/IRenderer.hpp>
#include <Renderer/Opengl/OpenglRendererCommand.hpp>
#include <Renderer/Opengl/OpenglResourceManager.hpp>
#include <Renderer/Opengl/OpenglFrameBufferManager.hpp>
#include <memory>

namespace InputProcessor::Renderer::Opengl {
	class OpenglRenderer : public InputProcessor::Renderer::IRenderer {
	public:
		static OpenglRenderer* GetInstance();
		static void Free();

		void Config(RendererConfiguration configuration) override;
		IRendererCommand* GetRendererCommand() override;
		IResourceManager* GetResourceManager() override;
		IFrameBufferManager* GetFrameBufferManager() override;

		RendererConfiguration GetConfig() override;

	private:
		OpenglRenderer();
		~OpenglRenderer();

		RendererConfiguration mConfiguration;

		inline static OpenglRenderer* sInstance = nullptr;
	};
}
