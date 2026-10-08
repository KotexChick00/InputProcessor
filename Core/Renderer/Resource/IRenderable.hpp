#pragma once
#include <pch.h>
#include <Renderer/IRenderer.hpp>
#include <Renderer/Resource/IShader.hpp>

namespace CoreEngine::Renderer {

	// Interface chung cho bất kỳ thứ gì "vẽ được" trong 1 frame: model, particle system,
	// UI overlay, skybox... Pipeline chỉ cần biết IRenderable, không cần biết chi tiết
	// bên trong là RenderModel hay loại nào khác — đây là điểm gom cho multi-source submit.
	class CORE_API IRenderable {
	public:
		virtual ~IRenderable() = default;

		// shader truyền ngoài (không phải thuộc tính cố định của renderable), vì cùng
		// 1 renderable có thể cần vẽ bằng shader khác nhau tuỳ pass (geometry, shadow...).
		virtual void Draw(IRenderer* renderer, IShader* shader) = 0;
	};
}