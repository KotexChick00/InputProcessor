#pragma once
#include <iostream>
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

	class IResourceManager {
	public:
		virtual ~IResourceManager() = default;

		virtual IVertexBuffer* GetVertexBuffer(VertexBufferID vertexBufferId) = 0;
		virtual IIndexBuffer* GetIndexBuffer(IndexBufferID indexBufferId) = 0;
		virtual IShader* GetShader(ShaderID shaderId) = 0;
		virtual ITexture* GetTexture(TextureID textureId) = 0;
		virtual ICubeMap* GetCubeMap(CubeMapID cubeMap) = 0;
		virtual IUniformBuffer* GetUniformBuffer(UniformBufferID uniformBufferId) = 0;
	};
}