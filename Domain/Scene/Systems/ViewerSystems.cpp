#include <Scene/Systems/ViewerSystems.hpp>
#include <Scene/Systems/TransformSystem.hpp>
#include <Scene/Systems/CameraSystem.hpp>
#include <Scene/Systems/LightingSystem.hpp>
#include <Scene/Systems/RenderSystem.hpp>
#include <ECS/SystemManager.hpp>

namespace Domain::Scene {
	void AddViewerSystems(const CoreEngine::InputState* input, CoreEngine::IRenderer* renderer) {
		auto& systems = CoreEngine::ECS::SystemManager::GetInstance();

		systems.AddSystem<TransformSystem>();
		systems.AddSystem<CameraSystem>(input);
		systems.AddSystem<LightingSystem>();
		systems.AddDependency<LightingSystem, TransformSystem>();

		if (renderer) {
			systems.AddSystem<RenderSystem>(renderer);
			systems.AddDependency<RenderSystem, TransformSystem>();
			systems.AddDependency<RenderSystem, CameraSystem>();
			systems.AddDependency<RenderSystem, LightingSystem>();
		}
	}
}
