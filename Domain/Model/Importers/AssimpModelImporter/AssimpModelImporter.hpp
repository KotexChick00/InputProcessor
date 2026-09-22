#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Model/Importers/IModelImporter.hpp>

namespace Model {
	class AssimpModelImporter : public IModelImporter {
	public:
		RenderModel* Import(const char* file) override;

	private:
		void ProcessNode()
	};
}