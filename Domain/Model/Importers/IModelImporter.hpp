#pragma once
#include <vector>
#include <Model/Mesh.hpp>

namespace Model {
	class IModelImporter {
	public:
		virtual ~IModelImporter() = default;
		virtual std::vector<Mesh> Import(const char* file) = 0;
	};
}