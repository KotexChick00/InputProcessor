#pragma once
#include <iostream>
#include <Renderer/Resource/IShader.hpp>

namespace InputProcessor::Domain::Model {
	using namespace InputProcessor::Renderer::Resource;

	class IMaterial {
	public:
		virtual ~IMaterial() = default;
		virtual void Setup(IShader* shader) = 0;
	};
};