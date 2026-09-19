#include <Renderer/Opengl/OpenglResourceManager.hpp>
#include <Logger/Logger.hpp>
#include <string>
#include <fstream>

namespace CoreEngine::Renderer::Opengl {
	using namespace CoreEngine::Logger;

	OpenglResourceManager::~OpenglResourceManager() {
		IP_ENGINE_TRACE("OpenglResourceManager destroy resources");

		isOnFree = true;

		for (auto kv : mVertexBuffers) {
			delete kv.second;
		}

		for (auto kv : mIndexBuffers) {
			delete kv.second;
		}

		for (auto kv : mShaders) {
			delete kv.second;
		}

		for (auto kv : mTextures) {
			delete kv.second;
		}

		for (auto kv : mCubeMaps) {
			delete kv.second;
		}
		for (auto kv : mUniformBuffers) {
			delete kv.second;
		}
	}

	IVertexBuffer* OpenglResourceManager::GetVertexBuffer(VertexBufferID vertexBufferId) {
		if (mVertexBuffers.contains(vertexBufferId)) {
			IP_ENGINE_TRACE("OpenglResourceManager get vertex buffer with id: {}", vertexBufferId);
			return mVertexBuffers.at(vertexBufferId);
		}

		IP_ENGINE_WARN("OpenglResourceManager not found vertex buffer with id {}, return null", vertexBufferId);
		return nullptr;
	}

	IUniformBuffer* OpenglResourceManager::GetUniformBuffer(UniformBufferID uniformBufferId) {
		if (mUniformBuffers.contains(uniformBufferId)) {
			IP_ENGINE_TRACE("OpenglResourceManager get uniform buffer with id: {}", uniformBufferId);
			return mUniformBuffers.at(uniformBufferId);
		}

		IP_ENGINE_WARN("OpenglResourceManager not found uniform buffer with id {}, return null", uniformBufferId);
		return nullptr;
	}

	IIndexBuffer* OpenglResourceManager::GetIndexBuffer(IndexBufferID indexBufferId) {
		if (mIndexBuffers.contains(indexBufferId)) {
			IP_ENGINE_TRACE("OpenglResourceManager get vertex buffer with id: {}", indexBufferId);
			return mIndexBuffers.at(indexBufferId);
		}

		IP_ENGINE_WARN("OpenglResourceManager not found index buffer with id {}, return null", indexBufferId);
		return nullptr;
	}

	IShader* OpenglResourceManager::GetShader(ShaderID shaderId) {
		if (mShaders.contains(shaderId)) {
			IP_ENGINE_TRACE("OpenglResourceManager get shader with id: {}", shaderId);
			return mShaders.at(shaderId);
		}

		IP_ENGINE_WARN("OpenglResourceManager not found shader with id {}, return null", shaderId);
		return nullptr;
	}

	ITexture* OpenglResourceManager::GetTexture(TextureID textureId) {
		if (mTextures.contains(textureId)) {
			IP_ENGINE_TRACE("OpenglResourceManager get texture with id: {}", textureId);
			return mTextures.at(textureId);
		}

		IP_ENGINE_WARN("OpenglResourceManager not found texture with id {}, return null", textureId);
		return nullptr;
	}

	ICubeMap* OpenglResourceManager::GetCubeMap(CubeMapID cubeMapId) {
		if (mCubeMaps.contains(cubeMapId)) {
			IP_ENGINE_TRACE("OpenglResourceManager get cube map with id: {}", cubeMapId);
			return mCubeMaps.at(cubeMapId);
		}

		IP_ENGINE_WARN("OpenglResourceManager not found cube map with id {}, return null", cubeMapId);
		return nullptr;
	}

	void OpenglResourceManager::Free() {
		if (sInstance != nullptr) {
			delete sInstance;
			sInstance = nullptr;
		}
	}

	void OpenglResourceManager::InsertVertexBuffer(OpenglVertexBuffer* vertexBuffer) {
		if (vertexBuffer == nullptr) return;
		VertexBufferID id = vertexBuffer->GetVertexBufferId();
		if (mVertexBuffers.contains(id)) {
			IP_ENGINE_WARN("OpenglResourceManager vertex buffer with id {} already exists, skipping insert", id);
			return;
		}
		mVertexBuffers.insert({ id, vertexBuffer });
		IP_ENGINE_TRACE("OpenglResourceManager insert vertex buffer with id: {}", id);
	}

	void OpenglResourceManager::InsertIndexBuffer(OpenglIndexBuffer* openglIndexBuffer) {
		if (openglIndexBuffer == nullptr) return;
		IndexBufferID id = openglIndexBuffer->GetIndexBufferId();
		if (mIndexBuffers.contains(id)) {
			IP_ENGINE_WARN("OpenglResourceManager index buffer with id {} already exists, skipping insert", id);
			return;
		}
		mIndexBuffers.insert({ id, openglIndexBuffer });
		IP_ENGINE_TRACE("OpenglResourceManager insert index buffer with id: {}", id);
	}

	void OpenglResourceManager::InsertShader(OpenglShader* shader) {
		if (shader == nullptr) return;
		ShaderID id = shader->GetShaderId();
		if (mShaders.contains(id)) {
			IP_ENGINE_WARN("OpenglResourceManager shader with id {} already exists, skipping insert", id);
			return;
		}
		mShaders.insert({ id, shader });
		IP_ENGINE_TRACE("OpenglResourceManager insert shader with id: {}", id);
	}

	void OpenglResourceManager::InsertTexture(OpenglTexture* texture) {
		if (texture == nullptr) return;
		TextureID id = texture->GetTextureId();
		if (mTextures.contains(id)) {
			IP_ENGINE_WARN("OpenglResourceManager texture with id {} already exists, skipping insert", id);
			return;
		}
		mTextures.insert({ id, texture });
		IP_ENGINE_TRACE("OpenglResourceManager insert texture with id: {}", id);
	}

	void OpenglResourceManager::InsertCubeMap(OpenglCubeMap* cubeMap) {
		if (cubeMap == nullptr) return;
		CubeMapID id = cubeMap->GetCubeMapId();
		if (mCubeMaps.contains(id)) {
			IP_ENGINE_WARN("OpenglResourceManager cubemap with id {} already exists, skipping insert", id);
			return;
		}
		mCubeMaps.insert({ id, cubeMap });
		IP_ENGINE_TRACE("OpenglResourceManager insert cubemap with id: {}", id);
	}

	void OpenglResourceManager::InsertUniformBuffer(OpenglUniformBuffer* uniformBuffer) {
		if (uniformBuffer == nullptr) return;
		UniformBufferID id = uniformBuffer->GetUniformId();
		if (mUniformBuffers.contains(id)) {
			IP_ENGINE_WARN("OpenglResourceManager uniform buffer with id {} already exists, skipping insert", id);
			return;
		}
		mUniformBuffers.insert({ id, uniformBuffer });
		IP_ENGINE_TRACE("OpenglResourceManager insert uniform buffer with id: {}", id);
	}

	void OpenglResourceManager::RemoveUniformBuffer(UniformBufferID uniformBufferId) {
		if (isOnFree) return;
		if (mUniformBuffers.contains(uniformBufferId)) {
			mUniformBuffers.erase(uniformBufferId);
			IP_ENGINE_TRACE("OpenglResourceManager removed uniform buffer with id: {}", uniformBufferId);
			return;
		}
		IP_ENGINE_WARN("OpenglResourceManager cannot remove uniform buffer with id {} because it does not exist", uniformBufferId);
	}

	void OpenglResourceManager::RemoveVertexBuffer(VertexBufferID vertexBufferId) {
		if (isOnFree) return;
		if (mVertexBuffers.contains(vertexBufferId)) {
			mVertexBuffers.erase(vertexBufferId);
			IP_ENGINE_TRACE("OpenglResourceManager removed vertex buffer with id: {}", vertexBufferId);
			return;
		}
		IP_ENGINE_WARN("OpenglResourceManager cannot remove vertex buffer with id {} because it does not exist", vertexBufferId);
	}

	void OpenglResourceManager::RemoveIndexBuffer(IndexBufferID indexBufferId) {
		if (isOnFree) return;
		if (mIndexBuffers.contains(indexBufferId)) {
			mIndexBuffers.erase(indexBufferId);
			IP_ENGINE_TRACE("OpenglResourceManager removed index buffer with id: {}", indexBufferId);
			return;
		}
		IP_ENGINE_WARN("OpenglResourceManager cannot remove index buffer with id {} because it does not exist", indexBufferId);
	}

	void OpenglResourceManager::RemoveShader(ShaderID shaderId) {
		if (isOnFree) return;
		if (mShaders.contains(shaderId)) {
			mShaders.erase(shaderId);
			IP_ENGINE_TRACE("OpenglResourceManager removed shader with id: {}", shaderId);
			return;
		}
		IP_ENGINE_WARN("OpenglResourceManager cannot remove shader with id {} because it does not exist", shaderId);
	}

	void OpenglResourceManager::RemoveTexture(TextureID textureId) {
		if (isOnFree) return;
		if (mTextures.contains(textureId)) {
			mTextures.erase(textureId);
			IP_ENGINE_TRACE("OpenglResourceManager removed texture with id: {}", textureId);
			return;
		}
		IP_ENGINE_WARN("OpenglResourceManager cannot remove texture with id {} because it does not exist", textureId);
	}

	void OpenglResourceManager::RemoveCubeMap(CubeMapID cubeMapId) {
		if (isOnFree) return;
		if (mCubeMaps.contains(cubeMapId)) {
			mCubeMaps.erase(cubeMapId);
			IP_ENGINE_TRACE("OpenglResourceManager removed cubemap with id: {}", cubeMapId);
			return;
		}
		IP_ENGINE_WARN("OpenglResourceManager cannot remove cubemap with id {} because it does not exist", cubeMapId);
	}
}

