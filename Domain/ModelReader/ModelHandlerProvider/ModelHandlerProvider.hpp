#pragma once
#include <ModelReader/IModelReader.hpp>
#include <ModelReader/ModelHandlerProvider/BaseModelHandler.hpp>

namespace InputProcessor::Domain {
	class ModelHandlerProvider : public IModelReader {
	public:
		ModelHandlerProvider(BaseModelHandler* handler) : mHandler(handler) { }

		~ModelHandlerProvider() {
			delete mHandler;
		}

		IRenderable* ReadModel(const char* file) override;
	private:
		BaseModelHandler* mHandler = nullptr;
	};
}