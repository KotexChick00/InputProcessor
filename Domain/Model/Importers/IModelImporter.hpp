#pragma once
#include <vector>
#include <Model/RenderModel.hpp>

namespace Model {
	class IModelImporter {
	public:
		virtual ~IModelImporter() = default;
		virtual RenderModel* Import(const char* file) = 0;
	};
}