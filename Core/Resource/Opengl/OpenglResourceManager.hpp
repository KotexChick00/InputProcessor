#pragma once
#include <Resource/ResouceManager.hpp>
#include <Resource/Opengl/OpenglVertexBuffer.hpp>
#include <Resource/Opengl/OpenglIndexBuffer.hpp>
#include <Resource/Opengl/OpenglShader.hpp>
#include <Resource/Opengl/OpenglTexture.hpp>
#include <unordered_map>
#include <vector>

namespace InputProcessor::Resource::Opengl {
	class OpenglResourceManager : public InputProcessor::Resource::ResourceManager {
	public:
		~OpenglResourceManager();

	protected:
		IVertexBuffer* CreateVertexBufferImpl() override;
		IIndexBuffer* CreateIndexBufferImpl() override;
		IShader* GetShaderFromSourceImpl(ShaderData shaderData) override;
		IShader* GetShaderFromFileImpl(const char* file) override;
		IShader* GetShaderFromNameImpl(const std::string& name) override;
		ITexture* GetTextureFromFileImpl(const char* file) override;

	private:
		std::vector<IVertexBuffer*> mVertexBuffers;
		std::vector<IIndexBuffer*> mIndexBuffers;
		std::unordered_map<std::string, IShader*> mShaders;
		std::unordered_map<std::string, ITexture*> mTextures;
	};
}