#pragma once
#include "ModelFormatHandler.hpp"

namespace Model {
	class ObjModelFormatHandler : public ModelFormatHandler {
	public:
		std::vector<Mesh> Handle(const char* file) override;
	};
}