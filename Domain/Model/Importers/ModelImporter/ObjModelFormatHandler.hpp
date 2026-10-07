#pragma once
#include "ModelFormatHandler.hpp"

namespace Model {
	class ObjModelFormatHandler : public ModelFormatHandler {
	public:
		RenderModel* Handle(const char* file) override;
	};
}