#pragma once
#include <pch.h>
#include <Renderer/Resource/IShader.hpp>

namespace CoreEngine::Renderer {
	enum class CORE_API CubeMapMagFilterMethod {
		BiLinear,
		Nearest
	};

	enum class CORE_API CubeMapMinFilterMethod {
		BiLinear,
		Nearest,
		BiLinearMimapBiLinear,
		BiLinearMimapNearest,
		NearestMimapBiLinear,
		NearestMimapNearest
	};

	// Most of time use ClampToBorder because of hardware limitation
	enum class CORE_API CubeMapWrapMethod {
		Repeat = 0,
		MirroedRepeat = 1,
		ClampToEdge = 2,
		ClampToBorder = 3
	};

	struct CORE_API CubeMapConfiguration {
		CubeMapMinFilterMethod MinFilterMethod = CubeMapMinFilterMethod::BiLinear;
		CubeMapMagFilterMethod MagFilterMethod = CubeMapMagFilterMethod::BiLinear;
		
		CubeMapWrapMethod WrapSMethod = CubeMapWrapMethod::ClampToEdge;
		CubeMapWrapMethod WrapTMethod = CubeMapWrapMethod::ClampToEdge;
		CubeMapWrapMethod WrapRMethod = CubeMapWrapMethod::ClampToEdge;
	};

	typedef unsigned int CubeMapID;

	class CORE_API ICubeMap {
	public:
		virtual ~ICubeMap() = default;

		virtual void Config(const CubeMapConfiguration& configuration) = 0;
		virtual void SetCubeMap(const std::string& samplerName, IShader* shader) = 0;
		virtual void SetCubeMap(const std::string& samplerName, unsigned int textureIdx, IShader* shader) = 0;
	
		virtual CubeMapID GetCubeMapId() const = 0;
	};
}