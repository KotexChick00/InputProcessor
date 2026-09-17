#pragma once
#include <iostream>
#include <glad/glad.h>
#include <Renderer/Resource/IIndexBuffer.hpp>

namespace InputProcessor::Renderer::Resource::Opengl {
class OpenglIndexBuffer : public InputProcessor::Renderer::Resource::IIndexBuffer {
	public:
		~OpenglIndexBuffer();

		void SetData(
			unsigned int* data,
			unsigned int size,
			IndexBufferRenderMode renderMode = IndexBufferRenderMode::Static
		) override;

		void Bind() override;
		void UnBind() override;

		IndexBufferID GetIndexBufferId() const override;

	private:
		OpenglIndexBuffer(GLuint mEboId);

	private:
		GLuint mEboId;

	public:
		static OpenglIndexBuffer* Create();
	};
}
