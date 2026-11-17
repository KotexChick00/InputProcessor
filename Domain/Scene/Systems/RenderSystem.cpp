#include <Scene/Systems/RenderSystem.hpp>
#include <Scene/Systems/CameraSystem.hpp>
#include <Scene/Systems/LightingSystem.hpp>
#include <Scene/Systems/ViewerShader.hpp>
#include <ECS/ComponentManager.hpp>
#include <ECS/SystemManager.hpp>
#include <Logger/Logger.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <string>

namespace Domain::Scene {
	void RenderSystem::OnCreate() {
		if (!mRenderer || !mRenderer->GetResourceManager()) {
			IP_ENGINE_ERROR("RenderSystem: no renderer, nothing will be drawn");
			return;
		}
		mShader = mRenderer->GetResourceManager()->CreateShaderFromSources(
			ViewerShader::VERTEX_SOURCE, ViewerShader::FRAGMENT_SOURCE);
		if (mShader && mShader->GetShaderId() == 0) mShader = nullptr;
		if (!mShader) IP_ENGINE_ERROR("RenderSystem: failed to create the viewer shader");
	}

	void RenderSystem::OnDestroy() {
		mShader = nullptr;
	}

	void RenderSystem::OnUpdate(float) {
		mLastDrawCount = 0;
		if (!mRenderer) return;

		auto* command = mRenderer->GetRendererCommand();
		command->ClearBuffers(CoreEngine::ClearBufferMasks::Color | CoreEngine::ClearBufferMasks::Depth);

		CameraComponent* camera = CameraSystem::GetActiveCamera();
		if (!mShader || !camera) return;

		auto& components = CoreEngine::ECS::ComponentManager::GetInstance();
		RenderSettingsComponent settings;
		for (const RenderSettingsComponent& found : components.GetAllComponents<RenderSettingsComponent>()) {
			settings.Wireframe = found.Wireframe;
			settings.Textures = found.Textures;
			settings.Lighting = found.Lighting;
			break;
		}

		mShader->Use();
		const glm::mat4 viewProjection = camera->Camera.GetProjectionMatrix() * camera->Camera.GetViewMatrix();
		mShader->SetUniformMatrix4fv("uViewProjection", glm::value_ptr(viewProjection));
		mShader->SetUniform1i("uEnableTextures", settings.Textures);
		mShader->SetUniform1i("uEnableLighting", settings.Lighting);
		if (settings.Lighting) UploadLights();

		command->SetPolygonMode(settings.Wireframe ? CoreEngine::PolygonMode::Line : CoreEngine::PolygonMode::Fill);

		for (MeshRendererComponent& renderer : components.GetAllComponents<MeshRendererComponent>()) {
			if (!renderer.Visible || !renderer.Model) continue;

			auto* transform = components.GetComponentByEntityID<TransformComponent>(renderer.GetOwner());
			const glm::mat4 model = transform ? transform->World : glm::mat4(1.0f);
			const glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

			mShader->Use();
			mShader->SetUniformMatrix4fv("uModel", glm::value_ptr(model));
			mShader->SetUniformMatrix3fv("uNormalMatrix", glm::value_ptr(normalMatrix));
			renderer.Model->Render(mRenderer, mShader);
			++mLastDrawCount;
		}

		// Trả lại chế độ mặc định để phần vẽ khác (UI...) không bị ảnh hưởng.
		command->SetPolygonMode(CoreEngine::PolygonMode::Fill);
	}

	void RenderSystem::UploadLights() {
		auto* lighting = CoreEngine::ECS::SystemManager::GetInstance().GetSystem<LightingSystem>();
		const size_t count = lighting ? lighting->GetLights().size() : 0;
		mShader->SetUniform1i("uLightCount", static_cast<int>(count));

		for (size_t i = 0; i < count; ++i) {
			const ShaderLight& light = lighting->GetLights()[i];
			const std::string prefix = "uLights[" + std::to_string(i) + "].";
			mShader->SetUniform1i(prefix + "type", static_cast<int>(light.Type));
			mShader->SetUniform3f(prefix + "position", light.Position.x, light.Position.y, light.Position.z);
			mShader->SetUniform3f(prefix + "direction", light.Direction.x, light.Direction.y, light.Direction.z);
			mShader->SetUniform3f(prefix + "color", light.Color.r, light.Color.g, light.Color.b);
			mShader->SetUniform1f(prefix + "cosInner", light.CosInnerCone);
			mShader->SetUniform1f(prefix + "cosOuter", light.CosOuterCone);
		}
	}
}
