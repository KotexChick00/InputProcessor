#pragma once
#ifdef _USE_GLAD_OPENGL
#include <glad/glad.h>
#endif
#include <Resource/IVertexBuffer.hpp>

namespace InputProcessor::Resource::Opengl {
	class OpenglConstantFactory {
	public:
		static GLenum ToGLRenderMode(RenderMode mode);
		static GLenum ToGLType(BufferDataType type);
	};
}