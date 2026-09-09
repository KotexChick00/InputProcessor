#include <Resource/Opengl/OpenglIndexBuffer.hpp>
#include <Resource/Opengl/OpenglConstantFactory.hpp>
#include <Logger/Logger.hpp>

namespace InputProcessor::Resource::Opengl {
	using namespace InputProcessor::Logger;

	OpenglIndexBuffer::OpenglIndexBuffer() : mEboId(0l) {
		glGenBuffers(1, &mEboId);
		if (mEboId == 0) {
			IP_ENGINE_ERROR("Failed to create element buffer object");
			return;
		}

		IP_ENGINE_TRACE("Created Element Buffer Object with id: {}", mEboId);
	}

	OpenglIndexBuffer::~OpenglIndexBuffer() {
		glDeleteBuffers(1, &mEboId);
		IP_ENGINE_TRACE("Destroyed Element Buffer Object with id: {}", mEboId);
	}

	void OpenglIndexBuffer::SetData(
		unsigned int* data,
		unsigned int size,
		RenderMode renderMode
	) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEboId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, OpenglConstantFactory::ToGLRenderMode(renderMode));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void OpenglIndexBuffer::Bind() {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEboId);
	}
	void OpenglIndexBuffer::UnBind() {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}