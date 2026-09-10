#include <ModelReader/ModelHandlerProvider/ModelHandlerProvider.hpp>
#include <Logger/Logger.hpp>

namespace InputProcessor::Domain {
	using namespace InputProcessor::Logger;

	IRenderable* ModelHandlerProvider::ReadModel(const char* file) {
		IRenderable* renderable = mHandler->Handle(file);
		if (renderable != nullptr) return renderable;

		IP_CLIENT_WARN("{} file don't have any handler, return null", file);
		return nullptr;
	}
}