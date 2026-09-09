#pragma once
#include <iostream>
#include <Resource/IVertexBuffer.hpp>

namespace InputProcessor::Resource {
	class IIndexBuffer {
	public:
		virtual ~IIndexBuffer() = default;

		virtual void SetData(
			unsigned int* data,
			unsigned int size,
			RenderMode renderMode = RenderMode::Static
		) = 0;

		virtual void Bind() = 0;
		virtual void UnBind() = 0;
	};
}