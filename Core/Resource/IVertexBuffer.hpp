#pragma once
#include <iostream>

namespace InputProcessor::Resource {
	enum class RenderMode {
		Static = 0,
		Stream = 1,
		Dynamic = 2
	};

	enum class BufferDataType {
		Float = 0,
		Int = 1,
		UnsignedInt = 2,
		Byte = 3
	};

	class IVertexBuffer {
	public:
		virtual ~IVertexBuffer() = default;

		virtual void SetData(
			unsigned int location, 
			void* data, 
			unsigned int size,
			unsigned int count,
			unsigned int stride,
			BufferDataType dataType = BufferDataType::Float,
			RenderMode renderMode = RenderMode::Static
		) = 0;

		virtual void SetData(
			unsigned int location,
			void* data,
			unsigned int size,
			unsigned int count,
			unsigned int stride,
			unsigned int offset,
			BufferDataType dataType = BufferDataType::Float,
			RenderMode renderMode = RenderMode::Static
		) = 0;

		virtual void Bind() = 0;
		virtual void UnBind() = 0;

		virtual void EnableAttrib(unsigned int indx) = 0;
		virtual void DisableAttrib(unsigned int indx) = 0;
	};
}