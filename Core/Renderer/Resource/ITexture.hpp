#pragma once
#include <iostream>
#include <Renderer/Resource/IShader.hpp>

namespace InputProcessor::Renderer::Resource {
	enum class WrapMethod {
		Repeat = 0,
		MirroedRepeat = 1,
		ClampToEdge = 2,
		ClampToBorder = 3
	};

	enum class MinFilterMethod {
		BiLinear,
		Nearest,
		BiLinearMimapBiLinear,
		BiLinearMimapNearest,
		NearestMimapBiLinear,
		NearestMimapNearest
	};

	enum class MagFilterMethod {
		BiLinear,
		Nearest
	};

	struct TextureConfiguration {
		WrapMethod WrapSMethod;
		WrapMethod WrapTMethod;
		MinFilterMethod MinFilter;
		MagFilterMethod MagFilter;

		TextureConfiguration(
			WrapMethod wrapSMethod,
			WrapMethod wrapTMethod,
			MinFilterMethod minFilter,
			MagFilterMethod magFilter
		) : WrapSMethod(wrapSMethod), WrapTMethod(wrapTMethod), MinFilter(minFilter), MagFilter(magFilter) { }

		TextureConfiguration() : 
			WrapSMethod(WrapMethod::Repeat), 
			WrapTMethod(WrapMethod::Repeat), 
			MinFilter(MinFilterMethod::BiLinearMimapBiLinear), 
			MagFilter(MagFilterMethod::BiLinear) {}
	};

	typedef unsigned int TextureID;

	class ITexture {
	public:
		virtual ~ITexture() = default;

		virtual void Config(const TextureConfiguration& config) = 0;
	
		virtual TextureID GetTextureId() const = 0;
	};
}