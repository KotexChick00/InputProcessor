#pragma once
#include <Input/InputState.hpp>
#include <Renderer/IRenderer.hpp>

namespace Domain::Scene {
	// Đăng ký các system của viewer vào ECS cùng thứ tự chạy:
	//   Transform, Camera -> Lighting -> Render
	// renderer == nullptr: bỏ qua RenderSystem (test không có OpenGL).
	// Gọi một lần sau khi renderer đã sẵn sàng; gỡ bằng CoreEngine::ECS::Shutdown().
	void AddViewerSystems(const CoreEngine::InputState* input, CoreEngine::IRenderer* renderer);
}
