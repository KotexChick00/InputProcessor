#pragma once
#include <iostream>
#include <Renderer/Resource/IVertexBuffer.hpp>

namespace CoreEngine::Renderer {
	enum class IndexBufferRenderMode {
		Static = 0,
		Stream = 1,
		Dynamic = 2
	};

	typedef unsigned int IndexBufferID;

	class IIndexBuffer {
	public:
		virtual ~IIndexBuffer() = default;

		virtual void SetData(
			unsigned int* data,
			unsigned int size,
			IndexBufferRenderMode renderMode = IndexBufferRenderMode::Static
		) = 0;

		virtual void Bind() = 0;
		virtual void UnBind() = 0;
	
		virtual IndexBufferID GetIndexBufferId() const = 0;
	};
}