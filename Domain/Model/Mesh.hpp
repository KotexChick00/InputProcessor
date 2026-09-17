#pragma once
#include <iostream>
#include <Model/IMaterial.hpp>
#include <vector>
#include <glm/glm.hpp>

namespace InputProcessor::Domain::Model {
	struct Vertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TextureCoords;
		glm::vec3 Color = glm::vec3(1.0f);
	};

	struct Mesh {
		std::vector<Vertex> Vertices;
		std::vector<unsigned int> Indices;
		IMaterial* Material;

		~Mesh() {
			delete Material;
		}
	};
}