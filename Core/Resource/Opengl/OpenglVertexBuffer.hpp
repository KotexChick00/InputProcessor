#pragma once
#include <iostream>
#include <unordered_map>
#include <glad/glad.h>
#include <Resource/IVertexBuffer.hpp>

namespace InputProcessor::Resource::Opengl {
	class OpenglVertexBuffer : public InputProcessor::Resource::IVertexBuffer {
	public:
		OpenglVertexBuffer();
		~OpenglVertexBuffer();

		void SetData(
			unsigned int location,
			void* data,
			unsigned int size,
			unsigned int count,
			unsigned int stride,
			BufferDataType dataType,
			RenderMode renderMode = RenderMode::Static
		) override;

		void SetData(
			unsigned int location,
			void* data,
			unsigned int size,
			unsigned int count,
			unsigned int stride,
			unsigned int offset,
			BufferDataType dataType,
			RenderMode renderMode = RenderMode::Static
		) override;

		void Bind() override;
		void UnBind() override;

		void EnableAttrib(unsigned int indx) override;
		void DisableAttrib(unsigned int indx) override;

	private:
		unsigned int mVaoID;
		std::unordered_map<unsigned int, unsigned int> mVboIds; // Mapping from location to vbo id
	};
}