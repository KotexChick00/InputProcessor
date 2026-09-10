#pragma once
#include <iostream>
#include <glm/glm.hpp>

namespace InputProcessor::Domain {
	struct Camera {
		glm::vec3 Position;
		glm::vec3 WorldUp;

	};
}