#pragma once
#include <iostream>
#include <Renderer/IRendererCommand.hpp>
#include <glad/glad.h>


namespace InputProcessor::Renderer::Opengl {
	class OpenglRendererCommand : public InputProcessor::Renderer::IRendererCommand {
	public:
		OpenglRendererCommand() = default;
		~OpenglRendererCommand() = default;

		void SetClearColor(ColorRGBA color) override;
		void ClearBuffers(BufferFlag bufferFlag) override;
		void Draw(RenderMode renderMode, unsigned int count, unsigned int start = 0) override;
		void DrawIndex(RenderMode renderMode, unsigned int count, unsigned int start = 0) override;
	};
}