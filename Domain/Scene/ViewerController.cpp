#include <Scene/ViewerController.hpp>
#include <Scene/Utf8Path.hpp>
#include <Logger/Logger.hpp>

#include <exception>
#include <filesystem>
#include <stdexcept>

namespace Domain::Scene {
	ViewerController::ViewerController(CoreEngine::IResourceManager* resources, CameraSystem* cameras)
		: mCameras(cameras)
		, mImporter(std::make_unique<Model::AssimpModelImporter>(resources))
		, mScene(std::make_unique<Scene>()) {
		if (!mCameras) throw std::invalid_argument("ViewerController needs the CameraSystem (call AddViewerSystems first)");
	}

	// Huỷ scene (giải phóng buffer GPU) khi OpenGL context còn sống: huỷ controller trước khi đóng cửa sổ.
	ViewerController::~ViewerController() = default;

	namespace {
		// Mọi API của controller trả lỗi qua `error` thay vì ném exception lên vòng lặp ứng dụng
		// (vd đổi đường dẫn trên Windows có thể ném std::system_error).
		template <typename Function>
		bool Guard(const char* action, std::string& error, Function&& function) {
			try {
				return function();
			} catch (const std::exception& exception) {
				error = std::string(action) + ": " + exception.what();
			} catch (...) {
				error = std::string(action) + ": unknown error";
			}
			IP_ENGINE_ERROR("ViewerController: {}", error);
			return false;
		}
	}

	bool ViewerController::OpenModel(const std::string& path, std::string& error) {
		return Guard("open model", error, [&] { return OpenModelUnguarded(path, error); });
	}

	bool ViewerController::OpenModelUnguarded(const std::string& path, std::string& error) {
		SceneLoader loader(*mImporter);
		Actor root = loader.LoadModel(*mScene, path);
		if (!root.IsValid()) {
			error = "can't read model '" + path + "'";
			return false;
		}
		mRoot = root;
		mModelPath = path;
		++mGeneration;
		auto* bounds = root.GetComponent<BoundsComponent>();
		mCameras->CreateCamera(*mScene, bounds ? &bounds->Box : nullptr);
		return true;
	}

	bool ViewerController::SaveView(const std::string& file, std::string& error) const {
		return Guard("save view", error, [&] {
			if (!HasModel()) {
				error = "no model is open";
				return false;
			}
			const std::filesystem::path viewFile = PathFromUtf8(file);
			const ViewState state = CaptureViewState(*mScene, mRoot, CameraSystem::GetActiveCamera(), viewFile);
			return SaveViewState(state, viewFile, error);
		});
	}

	bool ViewerController::LoadView(const std::string& file, std::string& error) {
		return Guard("load view", error, [&] {
			const std::filesystem::path viewFile = PathFromUtf8(file);
			auto state = LoadViewState(viewFile, error);
			if (!state) return false;
			const std::string model = PathToUtf8(ResolveModelPath(*state, viewFile));
			if (!OpenModelUnguarded(model, error)) return false;
			ApplyViewState(*state, *mScene, mRoot, CameraSystem::GetActiveCamera());
			return true;
		});
	}

	std::vector<EntityInfo> ViewerController::GetEntityTree() const {
		std::vector<EntityInfo> tree;
		if (HasModel()) CollectTree(mRoot, 0, tree);
		return tree;
	}

	void ViewerController::CollectTree(Actor entity, int depth, std::vector<EntityInfo>& out) const {
		EntityInfo info;
		info.Id = entity.GetID();
		info.Depth = depth;
		info.Name = entity.GetComponent<NameComponent>()->Name;
		info.Path = GetNodePath(entity, mRoot).value_or("");
		auto* hierarchy = entity.GetComponent<HierarchyComponent>();
		info.Parent = depth == 0 ? INVALID_ENTITY_ID : hierarchy->Parent;

		if (auto* renderer = entity.GetComponent<MeshRendererComponent>(); renderer && renderer->Model) {
			info.Visible = renderer->Visible;
			for (const auto& mesh : renderer->Model->GetMeshes()) {
				++info.MeshCount;
				info.VertexCount += mesh.Vertices.size();
				info.TriangleCount += mesh.Indices.size() / 3;
			}
		}
		if (auto* light = entity.GetComponent<LightComponent>()) {
			info.HasLight = true;
			info.LightType = light->Light.Type;
			info.LightEnabled = light->Enabled;
		}
		out.push_back(std::move(info));

		for (EntityID child : hierarchy->Children) {
			Actor childActor = CoreEngine::ECS::GetActor(child);
			if (childActor.IsValid()) CollectTree(childActor, depth + 1, out);
		}
	}

	ModelStats ViewerController::GetModelStats() const {
		ModelStats stats;
		if (!HasModel()) return stats;
		stats.Path = mModelPath;
		for (const EntityInfo& info : GetEntityTree()) {
			++stats.NodeCount;
			stats.MeshCount += info.MeshCount;
			stats.VertexCount += info.VertexCount;
			stats.TriangleCount += info.TriangleCount;
			if (info.HasLight) ++stats.LightCount;
		}
		if (auto* bounds = mRoot.GetComponent<BoundsComponent>(); bounds && bounds->Box.isValid()) {
			stats.HasBounds = true;
			stats.BoundsMin = bounds->Box.getMin();
			stats.BoundsMax = bounds->Box.getMax();
		}
		return stats;
	}

	bool ViewerController::IsInModel(Actor entity) const {
		return entity.IsValid() && HasModel() && GetNodePath(entity, mRoot).has_value();
	}

	bool ViewerController::SetVisible(EntityID id, bool visible, bool recursive) {
		Actor entity = CoreEngine::ECS::GetActor(id);
		if (!IsInModel(entity)) return false;
		SetVisibleRecursive(entity, visible, recursive);
		return true;
	}

	void ViewerController::SetVisibleRecursive(Actor entity, bool visible, bool recursive) {
		if (auto* renderer = entity.GetComponent<MeshRendererComponent>()) renderer->Visible = visible;
		if (!recursive) return;
		for (EntityID child : entity.GetComponent<HierarchyComponent>()->Children) {
			Actor childActor = CoreEngine::ECS::GetActor(child);
			if (childActor.IsValid()) SetVisibleRecursive(childActor, visible, true);
		}
	}

	bool ViewerController::SetLightEnabled(EntityID id, bool enabled) {
		Actor entity = CoreEngine::ECS::GetActor(id);
		auto* light = IsInModel(entity) ? entity.GetComponent<LightComponent>() : nullptr;
		if (!light) return false;
		light->Enabled = enabled;
		return true;
	}

	void ViewerController::FitCamera() {
		CameraComponent* camera = CameraSystem::GetActiveCamera();
		auto* bounds = HasModel() ? mRoot.GetComponent<BoundsComponent>() : nullptr;
		if (camera && bounds) mCameras->FitToBounds(*camera, bounds->Box);
	}
}
