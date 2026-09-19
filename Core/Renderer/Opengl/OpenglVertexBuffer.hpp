#pragma once
#include <Renderer/Resource/IVertexBuffer.hpp>
#include <glad/glad.h>
#include <unordered_map>

namespace CoreEngine::Renderer::Opengl {
	class OpenglVertexBuffer : public IVertexBuffer {
	public:
		~OpenglVertexBuffer();

		void SetData(
			unsigned int location,
			void* data,
			unsigned int size,
			unsigned int count,
			unsigned int stride,
			VertexBufferDataType dataType,
			VertexBufferRenderMode renderMode = VertexBufferRenderMode::Static
		) override;

		void SetData(
			unsigned int location,
			void* data,
			unsigned int size,
			unsigned int count,
			unsigned int stride,
			unsigned int offset,
			VertexBufferDataType dataType,
			VertexBufferRenderMode renderMode = VertexBufferRenderMode::Static
		) override;

		void Bind() override;
		void UnBind() override;

		void EnableAttrib(unsigned int indx) override;
		void DisableAttrib(unsigned int indx) override;

		VertexBufferID GetVertexBufferId() const override;

	private:
		OpenglVertexBuffer();

		GLuint mVaoID{0};
		std::unordered_map<unsigned int, GLuint> mVboIds;

	public:
		static OpenglVertexBuffer* Create();
	};
}
