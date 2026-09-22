#pragma once
#include <Model/Importers/IModelImporter.hpp>
#include <Model/Importers/ModelImporter/ModelFormatHandler.hpp>

namespace Model {
	class ModelImporter : public IModelImporter {
	public:
		ModelImporter(ModelFormatHandler* handler);
		~ModelImporter() = default;

		std::vector<Mesh> Import(const char* file) override;

	private:
		ModelFormatHandler* handler;
	};
}