#pragma once
#include <ECS/ISystem.hpp>
#include <Renderer/IRenderer.hpp>
#include <Scene/Components.hpp>

namespace Domain::Scene {
	// Vẽ một frame: xoá màn hình, đặt wireframe/texture/lighting theo RenderSettingsComponent,
	// truyền camera + đèn cho shader rồi vẽ mọi MeshRendererComponent đang Visible.
	// Cần OpenGL context: phải thêm system này sau khi cửa sổ/renderer đã khởi tạo.
	class RenderSystem : public CoreEngine::ECS::ISystem {
	public:
		explicit RenderSystem(CoreEngine::IRenderer* renderer)
			: ISystem(CoreEngine::ECS::ESystemPriority::Low), mRenderer(renderer) {}

		void OnCreate() override;
		void OnUpdate(float deltaTime) override;
		void OnDestroy() override;

		// false nếu không tạo được shader; khi đó OnUpdate chỉ xoá màn hình.
		bool IsReady() const { return mShader != nullptr; }
		// Số model đã vẽ ở frame gần nhất.
		size_t GetLastDrawCount() const { return mLastDrawCount; }

	private:
		void UploadLights();

	private:
		CoreEngine::IRenderer* mRenderer = nullptr;
		CoreEngine::IShader* mShader = nullptr; // Thuộc resource manager của renderer
		size_t mLastDrawCount = 0;
	};
}
