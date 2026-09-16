#include <Renderer/Opengl/OpenglUniformBuffer.hpp>
#include <Renderer/Opengl/OpenglResourceManager.hpp>
#include <Logger/Logger.hpp>

namespace InputProcessor::Renderer::Resource::Opengl {
	using namespace InputProcessor::Logger;

	OpenglUniformBuffer* OpenglUniformBuffer::Create() {
		GLuint uboId;
		glGenBuffers(1, &uboId);

		if (uboId == 0) {
			IP_ENGINE_WARN("Failed to create Uniform Buffer return nullptr");
			return nullptr;
		}

		OpenglUniformBuffer* newUniformBuffer = new OpenglUniformBuffer(uboId);
		OpenglResourceManager::GetInstance()->InsertUniformBuffer(newUniformBuffer);
		return newUniformBuffer;
	}

	OpenglUniformBuffer::OpenglUniformBuffer(GLuint uboId) : mBuffer(uboId) {
		mUniformId = static_cast<UniformBufferID>(mBuffer);
	}

	OpenglUniformBuffer::~OpenglUniformBuffer() {
		if (mBuffer != 0) {
			OpenglResourceManager::GetInstance()->RemoveUniformBuffer(GetUniformId());
			glDeleteBuffers(1, &mBuffer);
			mBuffer = 0;
		}
	}

	void OpenglUniformBuffer::Allocate(const size_t& memorySize) {
		mSize = memorySize;
		glBindBuffer(GL_UNIFORM_BUFFER, mBuffer);
		glBufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(mSize), nullptr, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void OpenglUniformBuffer::SetData(void* data, size_t size, UniformBufferRenderMode) {
		glBindBuffer(GL_UNIFORM_BUFFER, mBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, static_cast<GLsizeiptr>(size), data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void OpenglUniformBuffer::SetData(void* data, size_t size, size_t offset, UniformBufferRenderMode) {
		glBindBuffer(GL_UNIFORM_BUFFER, mBuffer);
		glBufferSubData(GL_UNIFORM_BUFFER, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void OpenglUniformBuffer::BindLocation(unsigned int bindingPointIdx) {
		if (mBuffer == 0) {
			IP_ENGINE_WARN("OpenglUniformBuffer::BindLocation called on empty buffer");
			return;
		}

		GLuint bindingPoint = static_cast<GLuint>(bindingPointIdx);
		// Bind the UBO to the specified binding point so shaders can access it
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, mBuffer);
	}

	UniformBufferID OpenglUniformBuffer::GetUniformId() const {
		return mUniformId;
	}
}
