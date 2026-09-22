#pragma once
#include <Renderer/Resource/ITexture.hpp>
#include <Renderer/Resource/IShader.hpp>

namespace Model {
	class IMaterial {
	public:
		virtual void Apply(CoreEngine::Renderer::IShader* shader);
	};
}