#include <Renderer/Opengl/OpenglIndexBuffer.hpp>
#include <Renderer/Opengl/OpenglConstantFactory.hpp>
#include <Renderer/Opengl/OpenglResourceManager.hpp>
#include <Logger/Logger.hpp>

namespace CoreEngine::Renderer::Opengl {
	using namespace CoreEngine::Logger;

	OpenglIndexBuffer* OpenglIndexBuffer::Create() {
		GLuint eboId;
		glGenBuffers(1, &eboId);

		if (eboId == 0) {
			IP_ENGINE_ERROR("Failed to create element buffer object");
			return nullptr;
		}

		IP_ENGINE_TRACE("Created Element Buffer Object with id: {}", eboId);
		OpenglIndexBuffer* newIndexBuffer = new OpenglIndexBuffer(eboId);
		OpenglResourceManager::GetInstance()->InsertIndexBuffer(newIndexBuffer);
		return newIndexBuffer;
	}

	OpenglIndexBuffer::OpenglIndexBuffer(GLuint eboId) : mEboId(eboId) { }

	OpenglIndexBuffer::~OpenglIndexBuffer() {
		OpenglResourceManager::GetInstance()->RemoveIndexBuffer(GetIndexBufferId());

		glDeleteBuffers(1, &mEboId);
		IP_ENGINE_TRACE("Destroyed Element Buffer Object with id: {}", mEboId);
	}

	void OpenglIndexBuffer::SetData(
		unsigned int* data,
		unsigned int size,
		IndexBufferRenderMode renderMode
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

	IndexBufferID OpenglIndexBuffer::GetIndexBufferId() const {
		return static_cast<IndexBufferID>(mEboId);
	}
}
