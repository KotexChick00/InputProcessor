#pragma once
#include <iostream>
#include <Renderer/IRendererCommand.hpp>
#include <glad/glad.h>


namespace InputProcessor::Renderer::Opengl {
	class OpenglRendererCommand : public IRendererCommand {
	public:
		static OpenglRendererCommand* GetInstance();
		static void Free();

		void ClearBuffers(ClearBufferMasks clearBufferMasks) override;
		void Draw(RenderMode renderMode, unsigned int count, unsigned int start = 0) override;
		void DrawIndex(RenderMode renderMode, unsigned int count, unsigned int start = 0) override;

	private:
		OpenglRendererCommand() = default;
		~OpenglRendererCommand() = default;

		inline static OpenglRendererCommand* sInstance = nullptr;
	};
}