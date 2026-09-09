#include <Resource/Opengl/OpenglConstantFactory.hpp>
#include <Logger/Logger.hpp>

namespace InputProcessor::Resource::Opengl {
	using namespace InputProcessor::Logger;

	GLenum OpenglConstantFactory::ToGLRenderMode(RenderMode mode) {
		switch (mode) {
			case RenderMode::Static:
				return GL_STATIC_DRAW;
			case RenderMode::Stream:
				return GL_STREAM_DRAW;
			case RenderMode::Dynamic:
				return GL_DYNAMIC_DRAW;
			default:
				IP_ENGINE_WARN("Undefined mode, return static mode instead");
				return GL_STATIC_DRAW;
		}
	}
	GLenum OpenglConstantFactory::ToGLType(BufferDataType type) {
		switch (type) {
			case BufferDataType::Float:
				return GL_FLOAT;
			case BufferDataType::Int:
				return GL_INT;
			case BufferDataType::UnsignedInt:
				return GL_UNSIGNED_INT;
			case BufferDataType::Byte:
				return GL_BYTE;
			default:
				IP_ENGINE_WARN("Undefined data type, return float instead");
				return GL_FLOAT;
		}
	}
}