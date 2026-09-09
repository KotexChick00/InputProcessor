#pragma once
#include <iostream>
#include <glad/glad.h>
#include <Resource/IIndexBuffer.hpp>

namespace InputProcessor::Resource::Opengl {
	class OpenglIndexBuffer : public InputProcessor::Resource::IIndexBuffer {
	public:
		OpenglIndexBuffer();
		~OpenglIndexBuffer();

		void SetData(
			unsigned int* data,
			unsigned int size,
			RenderMode renderMode = RenderMode::Static
		) override;

		void Bind() override;
		void UnBind() override;

	private:
		unsigned int mEboId;
	};
}