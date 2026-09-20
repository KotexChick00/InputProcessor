#pragma once
#include <pch.h>
#include <Logger/Logger.hpp>
#include <Renderer/Resource/IVertexBuffer.hpp>
#include <Renderer/Resource/IIndexBuffer.hpp>
#include <Renderer/Resource/ITexture.hpp>
#include <Renderer/Resource/IShader.hpp>
#include <Renderer/Resource/ICubeMap.hpp>
#include <Renderer/Resource/IUniformBuffer.hpp>
#include <Renderer/Resource/IFrameBuffer.hpp>

namespace CoreEngine::Renderer {
	using namespace CoreEngine::Logger;

	struct CubemapTextureFiles {
		std::string Front;
		std::string Back;
		std::string Bot;
		std::string Top;
		std::string Left;
		std::string Right;
	};

	class CORE_API IResourceManager {
	public:
		virtual ~IResourceManager() = default;

		virtual IVertexBuffer* GetVertexBuffer(VertexBufferID vertexBufferId) = 0;
		virtual IIndexBuffer* GetIndexBuffer(IndexBufferID indexBufferId) = 0;
		virtual IShader* GetShader(ShaderID shaderId) = 0;
		virtual ITexture* GetTexture(TextureID textureId) = 0;
		virtual ICubeMap* GetCubeMap(CubeMapID cubeMap) = 0;
		virtual IUniformBuffer* GetUniformBuffer(UniformBufferID uniformBufferId) = 0;

		virtual IVertexBuffer* CreateVertexBuffer() = 0;
		virtual IIndexBuffer* CreateIndexBuffer() = 0;
		virtual IShader* CreateShaderFromSources(const std::string& vertexSource, const std::string& fragmentSource) = 0;
		virtual IShader* CreateShaderFromFiles(const std::string& vertexFile, const std::string& fragmentFile) = 0;
		virtual ITexture* CreateTexture(const std::string& file) = 0;
		virtual ICubeMap* CreateCubeMap(const CubemapTextureFiles& textureFiles) = 0;
		virtual IUniformBuffer * CreateUniformBuffer() = 0;
	};
}