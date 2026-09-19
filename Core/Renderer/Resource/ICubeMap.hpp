#pragma once
#include <iostream>
#include <Renderer/Resource/IShader.hpp>

namespace CoreEngine::Renderer {
	enum class CubeMapMagFilterMethod {
		BiLinear,
		Nearest
	};

	enum class CubeMapMinFilterMethod {
		BiLinear,
		Nearest,
		BiLinearMimapBiLinear,
		BiLinearMimapNearest,
		NearestMimapBiLinear,
		NearestMimapNearest
	};

	// Most of time use ClampToBorder because of hardware limitation
	enum class CubeMapWrapMethod {
		Repeat = 0,
		MirroedRepeat = 1,
		ClampToEdge = 2,
		ClampToBorder = 3
	};

	struct CubeMapConfiguration {
		CubeMapMinFilterMethod MinFilterMethod = CubeMapMinFilterMethod::BiLinear;
		CubeMapMagFilterMethod MagFilterMethod = CubeMapMagFilterMethod::BiLinear;
		
		CubeMapWrapMethod WrapSMethod = CubeMapWrapMethod::ClampToEdge;
		CubeMapWrapMethod WrapTMethod = CubeMapWrapMethod::ClampToEdge;
		CubeMapWrapMethod WrapRMethod = CubeMapWrapMethod::ClampToEdge;
	};

	typedef unsigned int CubeMapID;

	class ICubeMap {
	public:
		virtual ~ICubeMap() = default;

		virtual void Config(const CubeMapConfiguration& configuration) = 0;
		virtual void SetCubeMap(const std::string& samplerName, IShader* shader) = 0;
		virtual void SetCubeMap(const std::string& samplerName, unsigned int textureIdx, IShader* shader) = 0;
	
		virtual CubeMapID GetCubeMapId() const = 0;
	};
}