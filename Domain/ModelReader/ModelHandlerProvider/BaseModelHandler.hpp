#pragma once
#include <iostream>
#include <IRenderable.hpp>

namespace InputProcessor::Domain {
	class BaseModelHandler {
	public:
		virtual ~BaseModelHandler() { delete mNextHandler; }

		virtual IRenderable* Handle(const char* file) = 0;
		void SetNextHandler(BaseModelHandler* handler) { mNextHandler = handler; }

	protected:
		IRenderable* Next(const char* file) { return mNextHandler != nullptr ? mNextHandler->Handle(file) : nullptr; }
	
	private:
		BaseModelHandler* mNextHandler = nullptr;
	};
}