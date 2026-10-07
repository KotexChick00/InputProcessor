#pragma once
#include "ModelFormatHandler.hpp"
#include <Model/Importers/AssimpModelImporter/AssimpModelImporter.hpp>
#include <Renderer/Resource/IResourceManager.hpp>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace Model {

	class GltfModelFormatHandler : public ModelFormatHandler {
	public:
		explicit GltfModelFormatHandler(CoreEngine::Renderer::IResourceManager* resourceManager)
			: mImporter(resourceManager)
		{}

		RenderModel* Handle(const char* file) override {
			if (!file || !*file) return nullptr;
			std::string extension = std::filesystem::path(file).extension().string();
			std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return std::tolower(c); });
			if (extension == ".gltf" || extension == ".glb") {
				auto result = mImporter.Import(file);
				return result.release();
			}
			return Next(file);
		}

	private:
		AssimpModelImporter mImporter;
	};

}
