#pragma once
#include <iostream>
#include <Model/IMaterial.hpp>
#include <Model/RenderModel.hpp>

namespace InputProcessor::Domain::Model::Importer {
	class IModelImporter {
	public:
		virtual ~IModelImporter() = default;
		virtual RenderModel* Import(const char* file) = 0;
	};
}