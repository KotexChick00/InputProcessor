#pragma once
#include <Renderer/RendererPrimitives.hpp>
#include <Renderer/IRendererCommand.hpp>
#include <Renderer/Resource/IResouceManager.hpp>
#include <Renderer/Resource/IFrameBufferManager.hpp>

namespace CoreEngine::Renderer {
	class IRenderer {
	public:
		virtual void Config(RendererConfiguration configuration) = 0;
		virtual IRendererCommand* GetRendererCommand() = 0;
		virtual IResourceManager* GetResourceManager() = 0;
		virtual IFrameBufferManager* GetFrameBufferManager() = 0;

		virtual RendererConfiguration GetConfig() = 0;
	};
}