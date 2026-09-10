#pragma once
#include <ModelReader/ModelHandlerProvider/BaseModelHandler.hpp>

namespace InputProcessor::Domain {
	class ObjModelHandler : public BaseModelHandler {
		IRenderable* Handle(const char* file) override;
	};
}