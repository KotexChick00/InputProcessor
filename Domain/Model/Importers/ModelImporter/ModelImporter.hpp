#pragma once
#include <Model/Importers/IModelImporter.hpp>
#include <Model/Importers/ModelImporter/ModelFormatHandler.hpp>

namespace Model {
	class ModelImporter : public IModelImporter {
	public:
		explicit ModelImporter(ModelFormatHandler* handler) : handler(handler) {}
		~ModelImporter() = default;

		std::unique_ptr<RenderModel> Import(const char* file) override {
			return std::unique_ptr<RenderModel>(file && handler ? handler->Handle(file) : nullptr);
		}

	private:
		ModelFormatHandler* handler;
	};
}
