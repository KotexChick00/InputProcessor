#pragma once
#include <pch.h>
#include <iostream>

namespace CoreEngine::Renderer {
	enum class CORE_API VertexBufferRenderMode {
		Static = 0,
		Stream = 1,
		Dynamic = 2
	};

	enum class CORE_API VertexBufferDataType {
		Float = 0,
		Int = 1,
		UnsignedInt = 2,
		Byte = 3
	};
	
	typedef unsigned int VertexBufferID;

	class CORE_API IVertexBuffer {
	public:
		virtual ~IVertexBuffer() = default;

		virtual void SetData(
			unsigned int location, 
			void* data, 
			unsigned int size,
			unsigned int count,
			unsigned int stride,
			VertexBufferDataType dataType = VertexBufferDataType::Float,
			VertexBufferRenderMode renderMode = VertexBufferRenderMode::Static
		) = 0;

		virtual void SetData(
			unsigned int location,
			void* data,
			unsigned int size,
			unsigned int count,
			unsigned int stride,
			unsigned int offset,
			VertexBufferDataType dataType = VertexBufferDataType::Float,
			VertexBufferRenderMode renderMode = VertexBufferRenderMode::Static
		) = 0;

		virtual void Bind() = 0;
		virtual void UnBind() = 0;

		virtual void EnableAttrib(unsigned int indx) = 0;
		virtual void DisableAttrib(unsigned int indx) = 0;

		virtual VertexBufferID GetVertexBufferId() const = 0;
	};
}