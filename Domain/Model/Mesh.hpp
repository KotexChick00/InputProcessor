#pragma once
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <Model/IMaterial.hpp>

namespace Model {
	struct Vertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TextureCoords;
		glm::vec3  Color;
	};

	struct Mesh {
		std::vector<Vertex> Vertices;
		std::vector<unsigned int> Indices;
		IMaterial* Material;
	};
}