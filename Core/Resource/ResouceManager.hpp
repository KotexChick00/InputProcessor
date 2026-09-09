#pragma once
#include <iostream>
#include <Logger/Logger.hpp>
#include <Resource/IVertexBuffer.hpp>
#include <Resource/IIndexBuffer.hpp>
#include <Resource/ITexture.hpp>
#include <Resource/IShader.hpp>

namespace InputProcessor::Resource {
	using namespace InputProcessor::Logger;

	struct ShaderData {
		std::string VertexSrc;
		std::string FragmentSrc;
		std::string Name;

		ShaderData() {}
		ShaderData(std::string vertexSrc, std::string fragmentSrc, std::string name): VertexSrc(vertexSrc), FragmentSrc(fragmentSrc), Name(name) {}
	};

	class ResourceManager {
	public:
		static void SetInstance(std::shared_ptr<ResourceManager> instance) { sInstance = instance; }

		static IVertexBuffer* CreateVertexBuffer() { 
			if (sInstance == nullptr) {
				IP_ENGINE_CRITICAL("ResourceManager instance haven't been set, return null");
				return nullptr;
			}

			IP_ENGINE_TRACE("ResourceManager created vertex buffer");
			return sInstance->CreateVertexBufferImpl();
		}

		static IIndexBuffer* CreateIndexBuffer() {
			if (sInstance == nullptr) {
				IP_ENGINE_CRITICAL("ResourceManager instance haven't been set, return null");
				return nullptr;
			}

			IP_ENGINE_TRACE("ResourceManager created index buffer");
			return sInstance->CreateIndexBufferImpl();
		}

		static IShader* GetShaderFromSource(ShaderData shaderData) {
			if (sInstance == nullptr) {
				IP_ENGINE_CRITICAL("ResourceManager instance haven't been set, return null");
				return nullptr;
			}

			IP_ENGINE_TRACE("ResourceManager get shader from source");
			return sInstance->GetShaderFromSourceImpl(shaderData);
		}

		static IShader* GetShaderFromFile(const char* file) {
			if (sInstance == nullptr) {
				IP_ENGINE_CRITICAL("ResourceManager instance haven't been set, return null");
				return nullptr;
			}

			IP_ENGINE_TRACE("ResourceManager get shader from file: {}", file);
			return sInstance->GetShaderFromFileImpl(file);
		}

		static IShader* GetShaderFromName(const std::string& name) {
			if (sInstance == nullptr) {
				IP_ENGINE_CRITICAL("ResourceManager instance haven't been set, return null");
				return nullptr;
			}

			IP_ENGINE_TRACE("ResourceManager get shader from name: {}", name);
			return sInstance->GetShaderFromNameImpl(name);
		}

		static ITexture* GetTextureFromFile(const char* file) {
			if (sInstance == nullptr) {
				IP_ENGINE_CRITICAL("ResourceManager instance haven't been set, return null");
				return nullptr;
			}

			IP_ENGINE_TRACE("ResourceManager get texture from file: {}", file);
			return sInstance->GetTextureFromFileImpl(file);
		}

	private:
		inline static std::shared_ptr<ResourceManager> sInstance = nullptr;

	protected:
		virtual IVertexBuffer* CreateVertexBufferImpl() = 0;
		virtual IIndexBuffer* CreateIndexBufferImpl() = 0;
		virtual IShader* GetShaderFromSourceImpl(ShaderData shaderData) = 0;
		virtual IShader* GetShaderFromFileImpl(const char* file) = 0;
		virtual IShader* GetShaderFromNameImpl(const std::string& name) = 0;
		virtual ITexture* GetTextureFromFileImpl(const char* file) = 0;
	};
}