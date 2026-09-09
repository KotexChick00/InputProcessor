#include <Renderer/Opengl/OpenglRendererCommand.hpp>
#include <Logger/Logger.hpp>

namespace InputProcessor::Renderer::Opengl {
	using namespace InputProcessor::Logger;

	void OpenglRendererCommand::SetClearColor(ColorRGBA color) {
		glClearColor(color.Red, color.Green, color.Blue, color.Alpha);
	}

	void OpenglRendererCommand::ClearBuffers(BufferFlag bufferFlag) {
		unsigned int bitWise = 0;
		if (HasFlag(bufferFlag, BufferFlag::Color)) {
			bitWise |= GL_COLOR_BUFFER_BIT;
		}

		if (HasFlag(bufferFlag, BufferFlag::Depth)) {
			bitWise |= GL_DEPTH_BUFFER_BIT;
		}

		if (bitWise == 0) {
			IP_ENGINE_ERROR("Undefined buffer flag, no clear buffers");
			return;
		}

		glClear(bitWise);
	}

	void OpenglRendererCommand::Draw(RenderMode renderMode, unsigned int count, unsigned int start) {
		unsigned int renderModeAddr = 0;
		switch (renderMode) {
			case RenderMode::Triangles:
				renderModeAddr = GL_TRIANGLES;
				break;
			default:
				IP_ENGINE_ERROR("Undefined renderMode, no action");
				break;
		}
		glDrawArrays(renderModeAddr, start, count);
	}

	void OpenglRendererCommand::DrawIndex(RenderMode renderMode, unsigned int count, unsigned int start) {
		unsigned int renderModeAddr = 0;
		switch (renderMode) {
		case RenderMode::Triangles:
			renderModeAddr = GL_TRIANGLES;
			break;
		default:
			IP_ENGINE_ERROR("Undefined renderMode, no action");
			break;
		}
		glDrawElements(renderModeAddr, count, GL_UNSIGNED_INT, (void*)start);
	}
}