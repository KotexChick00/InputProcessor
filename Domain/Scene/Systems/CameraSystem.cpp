#include <Scene/Systems/CameraSystem.hpp>
#include <ECS/ComponentManager.hpp>
#include <Logger/Logger.hpp>

#include <algorithm>

namespace Domain::Scene {
	namespace Orbit = Domain::Camera::OrbitCamera;

	namespace {
		constexpr float FIELD_OF_VIEW_DEGREES = 45.0f;
	}

	void CameraSystem::OnUpdate(float deltaTime) {
		CameraComponent* active = GetActiveCamera();
		if (!active || !mInput || !mInput->KeyboardInput || !mInput->MouseInput) return;

		if (!active->Controller) {
			active->Controller = std::make_unique<Orbit::OrbitCameraController>(active->Camera, *mInput);
		}
		active->Controller->Update(deltaTime);
	}

	void CameraSystem::SetViewportSize(uint32_t width, uint32_t height) {
		// Thu nhỏ cửa sổ về 0 thì giữ kích thước cũ để tránh chia cho 0 khi tính aspect.
		if (width == 0 || height == 0) return;
		mWidth = width;
		mHeight = height;
		auto& components = CoreEngine::ECS::ComponentManager::GetInstance();
		for (CameraComponent& camera : components.GetAllComponents<CameraComponent>()) {
			camera.Camera.SetViewportSize(width, height);
		}
	}

	Actor CameraSystem::CreateCamera(Scene& scene, const Model::AssimpBoundingBox* bounds) const {
		Actor entity = scene.CreateEntity("Camera");
		if (!entity.IsValid()) return {};

		auto* camera = entity.AddComponent<CameraComponent>();
		if (!camera) {
			IP_ENGINE_ERROR("CameraSystem::CreateCamera: out of component memory");
			scene.DestroyEntity(entity);
			return {};
		}

		camera->Camera.SetViewportSize(mWidth, mHeight);
		if (bounds && bounds->isValid()) FitToBounds(*camera, *bounds);
		return entity;
	}

	void CameraSystem::FitToBounds(CameraComponent& camera, const Model::AssimpBoundingBox& bounds) const {
		if (!bounds.isValid()) return;

		const float aspect = static_cast<float>(mWidth) / static_cast<float>(std::max(1u, mHeight));
		// Model cực nhỏ vẫn có bán kính > 0 để không chia cho 0 khi tính khoảng cách/near plane.
		const float scale = std::max(bounds.radius(), 0.001f);
		const Model::CameraSetup fit = Model::FitCameraToBox(bounds, FIELD_OF_VIEW_DEGREES, aspect);

		// Giống FitCamera() của viewer cũ để giữ nguyên góc nhìn ban đầu.
		camera.Camera = Orbit::OrbitCamera(fit.Target, 1.0f);
		camera.Camera.SetZoomLimits(scale * 0.01f, scale * 100.0f);
		camera.Camera.SetDistance(glm::length(fit.Position - fit.Target));
		camera.Camera.SetPerspective(glm::radians(FIELD_OF_VIEW_DEGREES), scale * 0.001f, scale * 110.0f);
		camera.Camera.SetViewportSize(mWidth, mHeight);
		camera.Camera.CaptureAsInitial();
	}

	CameraComponent* CameraSystem::GetActiveCamera() {
		auto& components = CoreEngine::ECS::ComponentManager::GetInstance();
		for (CameraComponent& camera : components.GetAllComponents<CameraComponent>()) {
			if (camera.Active) return &camera;
		}
		return nullptr;
	}
}
