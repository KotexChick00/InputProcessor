#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <Model/IMaterial.hpp>

namespace Model {
	struct Vertex {
		glm::vec3 Position;
		glm::vec3 Normal = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec2 TextureCoords = glm::vec2(0.0f, 0.0f);
		glm::vec3 Color = glm::vec3(1.0f, 1.0f, 1.0f);
	};

	struct Mesh {
		std::vector<Vertex> Vertices;
		std::vector<unsigned int> Indices;
		std::unique_ptr<IMaterial> Material;
	};
}