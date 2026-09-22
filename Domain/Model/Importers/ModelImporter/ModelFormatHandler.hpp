#pragma once
#include <vector>
#include <Model/Mesh.hpp>

namespace Model {
	class ModelFormatHandler {
	public:
		virtual std::vector<Mesh> Handle(const char* file) = 0;
		void SetNext(ModelFormatHandler* next) { mNext = next; }
		std::vector<Mesh> Next(const char* file) { mNext->Handle(file); }
	private:
		ModelFormatHandler* mNext = nullptr;
	};
}