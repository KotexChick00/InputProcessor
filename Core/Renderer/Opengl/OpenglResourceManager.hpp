#pragma once
#include <Renderer/Resource/IResouceManager.hpp>
#include <Renderer/Opengl/OpenglVertexBuffer.hpp>
#include <Renderer/Opengl/OpenglIndexBuffer.hpp>
#include <Renderer/Opengl/OpenglShader.hpp>
#include <Renderer/Opengl/OpenglTexture.hpp>
#include <Renderer/Opengl/OpenglCubeMap.hpp>
#include <Renderer/Opengl/OpenglUniformBuffer.hpp>
#include <unordered_map>
#include <vector>

namespace InputProcessor::Renderer::Resource::Opengl {
	class OpenglResourceManager : public InputProcessor::Renderer::Resource::IResourceManager {

	protected:
		OpenglResourceManager() = default;
		~OpenglResourceManager();

	public:
		static OpenglResourceManager* GetInstance() {
			if (sInstance == nullptr) {
				sInstance = new OpenglResourceManager();
			}

			return sInstance;
		}

		static void Free();

	public:
		IVertexBuffer* GetVertexBuffer(VertexBufferID vertexBufferId) override;
		IIndexBuffer* GetIndexBuffer(IndexBufferID indexBufferId) override;
		IShader* GetShader(ShaderID shaderId) override;
		ITexture* GetTexture(TextureID textureId) override;
		ICubeMap* GetCubeMap(CubeMapID cubeMapId) override;
		// Uniform buffer API
		IUniformBuffer* GetUniformBuffer(UniformBufferID uniformBufferId) override;

		void InsertVertexBuffer(OpenglVertexBuffer* vertexBuffer);
		void InsertIndexBuffer(OpenglIndexBuffer* openglIndexBuffer);
		void InsertShader(OpenglShader* shader);
		void InsertTexture(OpenglTexture* texture);
		void InsertCubeMap(OpenglCubeMap* cubeMap);
		void InsertUniformBuffer(OpenglUniformBuffer* uniformBuffer);

		void RemoveVertexBuffer(VertexBufferID vertexBufferId);
		void RemoveIndexBuffer(IndexBufferID indexBufferId);
		void RemoveShader(ShaderID shaderId);
		void RemoveTexture(TextureID textureId);
		void RemoveCubeMap(CubeMapID cubeMapId);
		void RemoveUniformBuffer(UniformBufferID uniformBufferId);

	private:
		std::unordered_map<VertexBufferID, OpenglVertexBuffer*> mVertexBuffers;
		std::unordered_map<IndexBufferID, OpenglIndexBuffer*> mIndexBuffers;
		std::unordered_map<ShaderID, OpenglShader*> mShaders;
		std::unordered_map<TextureID, OpenglTexture*> mTextures;
		std::unordered_map<CubeMapID, OpenglCubeMap*> mCubeMaps;
		std::unordered_map<UniformBufferID, OpenglUniformBuffer*> mUniformBuffers;

		bool isOnFree = false;

	private:
		inline static OpenglResourceManager* sInstance = nullptr;
	};
}
