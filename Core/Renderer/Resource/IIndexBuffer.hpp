#pragma once
#include <pch.h>
#include <Renderer/Resource/IVertexBuffer.hpp>

namespace CoreEngine::Renderer {
	enum class CORE_API IndexBufferRenderMode {
		Static = 0,
		Stream = 1,
		Dynamic = 2
	};

	typedef unsigned int IndexBufferID;

	class CORE_API IIndexBuffer {
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