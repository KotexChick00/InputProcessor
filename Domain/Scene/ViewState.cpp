#include <Scene/ViewState.hpp>
#include <Logger/Logger.hpp>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>

namespace Domain::Scene {
	using Json = nlohmann::json;

	namespace {
		Json Vec3ToJson(const glm::vec3& value) { return Json::array({ value.x, value.y, value.z }); }

		glm::vec3 Vec3FromJson(const Json& value) {
			if (!value.is_array() || value.size() != 3) throw std::invalid_argument("expected an array of 3 numbers");
			return { value.at(0).get<float>(), value.at(1).get<float>(), value.at(2).get<float>() };
		}

		// Tên của entity trong đường dẫn: tên node, thêm "#k" nếu là anh em trùng tên thứ k (k >= 1).
		std::string SegmentOf(Actor entity) {
			const std::string& name = entity.GetComponent<NameComponent>()->Name;
			auto* hierarchy = entity.GetComponent<HierarchyComponent>();
			Actor parent = CoreEngine::ECS::GetActor(hierarchy->Parent);
			if (!parent.IsValid()) return name;

			size_t occurrence = 0;
			for (EntityID sibling : parent.GetComponent<HierarchyComponent>()->Children) {
				if (sibling == entity.GetID()) break;
				Actor siblingActor = CoreEngine::ECS::GetActor(sibling);
				auto* siblingName = siblingActor.IsValid() ? siblingActor.GetComponent<NameComponent>() : nullptr;
				if (siblingName && siblingName->Name == name) ++occurrence;
			}
			return occurrence == 0 ? name : name + "#" + std::to_string(occurrence);
		}
	}

	std::string ViewStateToJson(const ViewState& state) {
		Json json;
		json["version"] = VIEW_STATE_VERSION;
		json["model"] = state.ModelPath;
		if (state.HasCamera) {
			json["camera"] = {
				{ "target", Vec3ToJson(state.CameraTarget) },
				{ "distance", state.CameraDistance },
				{ "azimuth", state.CameraAzimuth },
				{ "elevation", state.CameraElevation },
			};
		}
		json["render"] = { { "wireframe", state.Wireframe }, { "textures", state.Textures }, { "lighting", state.Lighting } };
		json["hiddenNodes"] = state.HiddenNodes;
		json["disabledLights"] = state.DisabledLights;
		return json.dump(2);
	}

	std::optional<ViewState> ViewStateFromJson(std::string_view text, std::string& error) {
		try {
			const Json json = Json::parse(text);
			if (!json.is_object()) throw std::invalid_argument("the root must be a JSON object");

			const int version = json.at("version").get<int>();
			if (version != VIEW_STATE_VERSION) {
				throw std::invalid_argument("unsupported version " + std::to_string(version)
					+ " (expected " + std::to_string(VIEW_STATE_VERSION) + ")");
			}

			ViewState state;
			state.ModelPath = json.at("model").get<std::string>();
			if (state.ModelPath.empty()) throw std::invalid_argument("\"model\" is empty");

			if (json.contains("camera")) {
				const Json& camera = json.at("camera");
				state.HasCamera = true;
				state.CameraTarget = Vec3FromJson(camera.at("target"));
				state.CameraDistance = camera.at("distance").get<float>();
				state.CameraAzimuth = camera.at("azimuth").get<float>();
				state.CameraElevation = camera.at("elevation").get<float>();
				if (!(state.CameraDistance > 0.0f)) throw std::invalid_argument("camera distance must be positive");
			}
			if (json.contains("render")) {
				const Json& render = json.at("render");
				state.Wireframe = render.value("wireframe", state.Wireframe);
				state.Textures = render.value("textures", state.Textures);
				state.Lighting = render.value("lighting", state.Lighting);
			}
			if (json.contains("hiddenNodes")) state.HiddenNodes = json.at("hiddenNodes").get<std::vector<std::string>>();
			if (json.contains("disabledLights")) state.DisabledLights = json.at("disabledLights").get<std::vector<std::string>>();
			return state;
		} catch (const std::exception& exception) {
			error = exception.what();
			return std::nullopt;
		}
	}

	bool SaveViewState(const ViewState& state, const std::filesystem::path& file, std::string& error) {
		std::ofstream stream(file, std::ios::binary | std::ios::trunc);
		if (!stream) {
			error = "can't open '" + file.string() + "' for writing";
			return false;
		}
		stream << ViewStateToJson(state) << '\n';
		if (!stream) {
			error = "can't write '" + file.string() + "'";
			return false;
		}
		return true;
	}

	std::optional<ViewState> LoadViewState(const std::filesystem::path& file, std::string& error) {
		std::ifstream stream(file, std::ios::binary);
		if (!stream) {
			error = "can't open '" + file.string() + "'";
			return std::nullopt;
		}
		std::stringstream content;
		content << stream.rdbuf();
		auto state = ViewStateFromJson(content.str(), error);
		if (!state) error = "'" + file.string() + "': " + error;
		return state;
	}

	std::optional<std::string> GetNodePath(Actor entity, Actor root) {
		std::vector<std::string> segments;
		for (Actor current = entity; current.IsValid(); ) {
			if (current == root) {
				std::string path;
				for (auto it = segments.rbegin(); it != segments.rend(); ++it) {
					if (!path.empty()) path += '/';
					path += *it;
				}
				return path;
			}
			segments.push_back(SegmentOf(current));
			current = CoreEngine::ECS::GetActor(current.GetComponent<HierarchyComponent>()->Parent);
		}
		return std::nullopt;
	}

	Actor FindByNodePath(Actor root, std::string_view path) {
		Actor current = root;
		while (current.IsValid() && !path.empty()) {
			const size_t slash = path.find('/');
			const std::string_view segment = path.substr(0, slash);
			path = slash == std::string_view::npos ? std::string_view() : path.substr(slash + 1);

			Actor next;
			for (EntityID child : current.GetComponent<HierarchyComponent>()->Children) {
				Actor childActor = CoreEngine::ECS::GetActor(child);
				if (childActor.IsValid() && SegmentOf(childActor) == segment) {
					next = childActor;
					break;
				}
			}
			current = next;
		}
		return current;
	}

	ViewState CaptureViewState(const Scene& scene, Actor modelRoot, const CameraComponent* camera,
		const std::filesystem::path& viewFile) {
		ViewState state;

		// Ghi đường dẫn model tương đối với thư mục file trạng thái (dễ chép cả thư mục sang máy khác).
		if (auto* source = modelRoot.IsValid() ? modelRoot.GetComponent<ModelSourceComponent>() : nullptr) {
			std::error_code code;
			const auto model = std::filesystem::absolute(source->Path, code);
			const auto base = std::filesystem::absolute(viewFile, code).parent_path();
			const auto relative = std::filesystem::relative(model, base, code);
			state.ModelPath = (!code && !relative.empty() ? relative : model).generic_string();
		}

		if (camera) {
			state.HasCamera = true;
			state.CameraTarget = camera->Camera.GetTarget();
			state.CameraDistance = camera->Camera.GetDistance();
			state.CameraAzimuth = camera->Camera.GetAzimuth();
			state.CameraElevation = camera->Camera.GetElevation();
		}

		const auto& settings = scene.GetRenderSettings();
		state.Wireframe = settings.Wireframe;
		state.Textures = settings.Textures;
		state.Lighting = settings.Lighting;

		for (const Actor& entity : scene.GetEntities()) {
			auto path = GetNodePath(entity, modelRoot);
			if (!path) continue; // không thuộc model (camera, đèn mặc định)
			if (auto* renderer = entity.GetComponent<MeshRendererComponent>(); renderer && !renderer->Visible) {
				state.HiddenNodes.push_back(*path);
			}
			if (auto* light = entity.GetComponent<LightComponent>(); light && !light->Enabled) {
				state.DisabledLights.push_back(*path);
			}
		}
		return state;
	}

	std::filesystem::path ResolveModelPath(const ViewState& state, const std::filesystem::path& viewFile) {
		std::filesystem::path model(state.ModelPath);
		if (model.is_relative()) model = viewFile.parent_path() / model;
		return model.lexically_normal();
	}

	size_t ApplyViewState(const ViewState& state, Scene& scene, Actor modelRoot, CameraComponent* camera) {
		auto& settings = scene.GetRenderSettings();
		settings.Wireframe = state.Wireframe;
		settings.Textures = state.Textures;
		settings.Lighting = state.Lighting;

		if (camera && state.HasCamera) {
			camera->Camera.SetTarget(state.CameraTarget);
			camera->Camera.SetDistance(state.CameraDistance);
			camera->Camera.SetAzimuth(state.CameraAzimuth);
			camera->Camera.SetElevation(state.CameraElevation);
		}

		size_t skipped = 0;
		for (const std::string& path : state.HiddenNodes) {
			Actor entity = FindByNodePath(modelRoot, path);
			auto* renderer = entity.IsValid() ? entity.GetComponent<MeshRendererComponent>() : nullptr;
			if (renderer) renderer->Visible = false;
			else { ++skipped; IP_ENGINE_WARN("ApplyViewState: no mesh node '{}' in the model, skip", path); }
		}
		for (const std::string& path : state.DisabledLights) {
			Actor entity = FindByNodePath(modelRoot, path);
			auto* light = entity.IsValid() ? entity.GetComponent<LightComponent>() : nullptr;
			if (light) light->Enabled = false;
			else { ++skipped; IP_ENGINE_WARN("ApplyViewState: no light '{}' in the model, skip", path); }
		}
		return skipped;
	}
}
