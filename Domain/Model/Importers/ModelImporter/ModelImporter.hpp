#pragma once
#include <Model/Importers/IModelImporter.hpp>
#include <Model/Importers/ModelImporter/ModelFormatHandler.hpp>

namespace Model {
	class ModelImporter : public IModelImporter {
	public:
		ModelImporter(ModelFormatHandler* handler);
		~ModelImporter() = default;

		RenderModel* Import(const char* file) override;

	private:
		ModelFormatHandler* handler;
	};
}