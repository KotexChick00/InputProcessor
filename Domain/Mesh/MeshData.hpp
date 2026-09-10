#pragma once
#include <iostream>
#include <vector>
#include <glm/glm.hpp>

namespace InputProcessor::Domain::Model {
	struct Vertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 UV;
	};

	struct MeshData {
		std::vector<Vertex> Vertices;
		std::vector<unsigned int> Indices;
	};
}