#pragma once
#include <vector>
#include <Model/RenderModel.hpp>

namespace Model {
	class ModelFormatHandler {
	public:
		virtual RenderModel* Handle(const char* file) = 0;
		void SetNext(ModelFormatHandler* next) { mNext = next; }
		RenderModel* Next(const char* file) { return mNext->Handle(file); }
	private:
		ModelFormatHandler* mNext = nullptr;
	};
}