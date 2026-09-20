#include <Renderer/Opengl/OpenglRendererCommand.hpp>
#include <Logger/Logger.hpp>

namespace CoreEngine::Renderer::Opengl {
	using namespace CoreEngine::Logger;

	OpenglRendererCommand* OpenglRendererCommand::GetInstance() {
		if (sInstance == nullptr) sInstance = new OpenglRendererCommand();
		return sInstance;
	}

	void OpenglRendererCommand::Free() {
		if (sInstance != nullptr) {
			delete sInstance;
			sInstance = nullptr;
		}
	}

	void OpenglRendererCommand::ClearBuffers(ClearBufferMasks clearBufferMasks) {
		unsigned int bitWise = 0;
		if (HasFlag(clearBufferMasks, ClearBufferMasks::Color)) {
			bitWise |= GL_COLOR_BUFFER_BIT;
		}

		if (HasFlag(clearBufferMasks, ClearBufferMasks::Depth)) {
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