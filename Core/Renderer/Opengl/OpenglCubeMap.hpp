#pragma once
#include <iostream>
#include <Renderer/Resource/ICubeMap.hpp>
#include <glad/glad.h>

namespace CoreEngine::Renderer::Opengl {
	struct CubeMapFiles {
		std::string Front;
		std::string Back;
		std::string Left;
		std::string Right;
		std::string Top;
		std::string Bottom;
	};

	class OpenglCubeMap : public ICubeMap {
	public:
		~OpenglCubeMap();

		void Config(const CubeMapConfiguration& configuration) override;
		void SetCubeMap(const std::string& samplerName, IShader* shader) override;
		void SetCubeMap(const std::string& samplerName, unsigned int textureIdx, IShader* shader) override;

		CubeMapID GetCubeMapId() const override;

	private:
		OpenglCubeMap(GLuint cubemapId);

		GLuint cubemapId;

	public:
		static OpenglCubeMap* FromFiles(CubeMapFiles cubemapFiles);
	};
}