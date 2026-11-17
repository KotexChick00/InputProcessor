#pragma once
#include <ECS/ECS.hpp>
#include <Scene/Components.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace Domain::Scene {
	using CoreEngine::ECS::Actor;

	// Danh sách entity của một scene, xây trên ECS (các manager của ECS là singleton toàn cục).
	// Scene chỉ quản lý những entity do nó tạo ra.
	// Luôn có sẵn một entity "Render Settings" mang RenderSettingsComponent; Clear() giữ lại entity này.
	class Scene {
	public:
		Scene();
		~Scene();

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		// Tạo entity với Name + Transform + Hierarchy. parent không hợp lệ -> entity gốc.
		// Trả về Actor không hợp lệ nếu hết bộ nhớ component.
		Actor CreateEntity(std::string name, Actor parent = {});

		// Huỷ entity cùng toàn bộ entity con. Không huỷ được entity Render Settings.
		void DestroyEntity(Actor entity);

		// Huỷ mọi entity trừ Render Settings (dùng khi mở model khác, giữ tuỳ chọn hiển thị).
		void Clear();

		const std::vector<Actor>& GetEntities() const { return mEntities; }
		std::vector<Actor> GetRoots() const;
		Actor FindByName(std::string_view name) const;

		Actor GetSettingsEntity() const { return mSettings; }
		RenderSettingsComponent& GetRenderSettings() const;

	private:
		void DestroyRecursive(Actor entity);

	private:
		std::vector<Actor> mEntities; // Không gồm mSettings
		Actor mSettings;
	};
}
