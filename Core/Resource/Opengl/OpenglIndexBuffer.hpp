#pragma once
#include <iostream>
#ifdef _USE_GLAD_OPENGL
#include <glad/glad.h>
#endif
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