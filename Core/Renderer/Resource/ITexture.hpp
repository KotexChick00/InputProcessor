#pragma once
#include <pch.h>
#include <Renderer/Resource/IShader.hpp>

namespace CoreEngine::Renderer {
	enum class CORE_API WrapMethod {
		Repeat = 0,
		MirroedRepeat = 1,
		ClampToEdge = 2,
		ClampToBorder = 3
	};

	enum class CORE_API MinFilterMethod {
		BiLinear,
		Nearest,
		BiLinearMimapBiLinear,
		BiLinearMimapNearest,
		NearestMimapBiLinear,
		NearestMimapNearest
	};

	enum class CORE_API MagFilterMethod {
		BiLinear,
		Nearest
	};

	struct CORE_API TextureConfiguration {
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

	class CORE_API ITexture {
	public:
		virtual ~ITexture() = default;

		virtual void Config(const TextureConfiguration& config) = 0;
	
		virtual TextureID GetTextureId() const = 0;
	};
}