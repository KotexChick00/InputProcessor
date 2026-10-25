#pragma once
#include <Renderer/RendererPrimitives.hpp>
#include <Renderer/IRendererCommand.hpp>
#include <Renderer/Resource/IResourceManager.hpp>
#include <Renderer/Resource/IFrameBufferManager.hpp>
#include <pch.h>

namespace CoreEngine::Renderer {
	class CORE_API IRenderer {
	public:
		virtual ~IRenderer() = default;
		virtual void Config(RendererConfiguration configuration) = 0;
		virtual IRendererCommand* GetRendererCommand() = 0;
		virtual IResourceManager* GetResourceManager() = 0;
		virtual IFrameBufferManager* GetFrameBufferManager() = 0;

		virtual void Free() = 0;

		virtual RendererConfiguration GetConfig() = 0;
	};
}
