#pragma once
#include <vector>
#include <memory>
#include <Model/RenderModel.hpp>

namespace Model {
	class IModelImporter {
	public:
		virtual ~IModelImporter() = default;
		virtual std::unique_ptr<RenderModel> Import(const char* file) = 0;
	};
}