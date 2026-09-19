#include <Renderer/Opengl/OpenglVertexBuffer.hpp>
#include <Renderer/Opengl/OpenglConstantFactory.hpp>
#include <Renderer/Opengl/OpenglResourceManager.hpp>
#include <Logger/Logger.hpp>

namespace CoreEngine::Renderer::Opengl {
	using namespace CoreEngine::Logger;

	OpenglVertexBuffer::OpenglVertexBuffer() {
		mVaoID = 0;
		glGenVertexArrays(1, &mVaoID);

		if (mVaoID == 0) {
			IP_ENGINE_ERROR("Can't generate Vertex Array Object");
			return;
		}

		IP_ENGINE_TRACE("Opengl Vertex Array created with id: {}", mVaoID);
	}

	OpenglVertexBuffer::~OpenglVertexBuffer() {
		OpenglResourceManager::GetInstance()->RemoveVertexBuffer(GetVertexBufferId());
		IP_ENGINE_TRACE("Opengl Vertex Array destroy with id: {}", mVaoID);

		for (std::unordered_map<unsigned int, unsigned int>::iterator it = mVboIds.begin(); it != mVboIds.end(); ++it) {
			unsigned int vboId = it->second;
			glDeleteBuffers(1, &vboId);
		}

		glDeleteVertexArrays(1, &mVaoID);
	}

	void OpenglVertexBuffer::SetData(
		unsigned int location,
		void* data,
		unsigned int size,
		unsigned int count,
		unsigned int stride,
		VertexBufferDataType dataType,
		VertexBufferRenderMode renderMode
	) {
		if (mVboIds.find(location) != mVboIds.end()) {
			GLuint vboId = mVboIds.at(location);
			glDeleteBuffers(1, &vboId);
		}

		GLuint newVboId = 0;
		glGenBuffers(1, &newVboId);
		if (newVboId == 0) {
			IP_ENGINE_ERROR("Can't create new Vertex Buffer Object");
			return;
		}

		glBindVertexArray(mVaoID);
		glBindBuffer(GL_ARRAY_BUFFER, newVboId);
		glBufferData(GL_ARRAY_BUFFER, size, data, OpenglConstantFactory::ToGLRenderMode(renderMode));

		glVertexAttribPointer(location, count, OpenglConstantFactory::ToGLType(dataType), GL_FALSE, stride, (void*)0);
		glEnableVertexAttribArray(location);
		glBindVertexArray(0);
	}

	void OpenglVertexBuffer::SetData(
		unsigned int location,
		void* data,
		unsigned int size,
		unsigned int count,
		unsigned int stride,
		unsigned int offset,
		VertexBufferDataType dataType,
		VertexBufferRenderMode renderMode
	) {
		if (mVboIds.find(location) != mVboIds.end()) {
			GLuint vboId = mVboIds.at(location);
			glDeleteBuffers(1, &vboId);
		}

		unsigned int newVboId = 0;
		glGenBuffers(1, &newVboId);
		if (newVboId == 0) {
			IP_ENGINE_ERROR("Can't create new Vertex Buffer Object");
			return;
		}

		glBindVertexArray(mVaoID);
		glBindBuffer(GL_ARRAY_BUFFER, newVboId);
		glBufferData(GL_ARRAY_BUFFER, size, data, OpenglConstantFactory::ToGLRenderMode(renderMode));

		glVertexAttribPointer(location, count, OpenglConstantFactory::ToGLType(dataType), GL_FALSE, stride, (void*)offset);
		glEnableVertexAttribArray(location);
		glBindVertexArray(0);
	}

	void OpenglVertexBuffer::Bind() {
		glBindVertexArray(mVaoID);
	}

	void OpenglVertexBuffer::UnBind() {
		glBindVertexArray(0);
	}

	void OpenglVertexBuffer::EnableAttrib(unsigned int indx) {
		glEnableVertexAttribArray(indx);
	}

	void OpenglVertexBuffer::DisableAttrib(unsigned int indx) {
		glDisableVertexAttribArray(indx);
	}

	VertexBufferID OpenglVertexBuffer::GetVertexBufferId() const {
		return static_cast<VertexBufferID>(mVaoID);
	}

	OpenglVertexBuffer* OpenglVertexBuffer::Create() {
		OpenglVertexBuffer* newVertexBuffer = new OpenglVertexBuffer();
		OpenglResourceManager::GetInstance()->InsertVertexBuffer(newVertexBuffer);
		return newVertexBuffer;
	}
}
