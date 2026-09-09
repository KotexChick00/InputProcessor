#pragma once
#include <glad/glad.h>
#include <Resource/IVertexBuffer.hpp>

namespace InputProcessor::Resource::Opengl {
	class OpenglConstantFactory {
	public:
		static GLenum ToGLRenderMode(RenderMode mode);
		static GLenum ToGLType(BufferDataType type);
	};
}