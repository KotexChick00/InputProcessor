#pragma once
#include <Renderer/Resource/IUniformBuffer.hpp>
#include <glad/glad.h>
#include <string>

namespace CoreEngine::Renderer::Opengl {
	class OpenglUniformBuffer : public IUniformBuffer {
	public:
		~OpenglUniformBuffer();

		void Allocate(const size_t& memorySize) override;
		void SetData(void* data, size_t size, UniformBufferRenderMode renderMode = UniformBufferRenderMode::Static) override;
		void SetData(void* data, size_t size, size_t offset, UniformBufferRenderMode renderMode = UniformBufferRenderMode::Static) override;

		void BindLocation(unsigned int bindingPointIdx) override;

		UniformBufferID GetUniformId() const override;

	private:
		GLuint mBuffer = 0;
		UniformBufferID mUniformId = 0;
		size_t mSize = 0;

	private:
		OpenglUniformBuffer(GLuint mBuffer);

	public:
		static OpenglUniformBuffer* Create();
	};
}
