#pragma once
#include <Renderer/Resource/ITexture.hpp>
#include <Renderer/Resource/IShader.hpp>

namespace Model {
	class IMaterial {
	public:
		virtual ~IMaterial() = default;
		virtual void Apply(CoreEngine::IShader* shader) = 0;
	};
}