#include <Resource/Opengl/OpenglResourceManager.hpp>
#include <Logger/Logger.hpp>
#include <string>
#include <fstream>

namespace InputProcessor::Resource::Opengl {
	using namespace InputProcessor::Logger;

	OpenglResourceManager::~OpenglResourceManager() {
		IP_ENGINE_TRACE("OpenglResourceManager destroy resources");
		for (auto vertexBuffer : mVertexBuffers) {
			delete vertexBuffer;
		}

		for (auto indexBuffer : mIndexBuffers) {
			delete indexBuffer;
		}

		for (auto kv : mShaders) {
			delete kv.second;
		}

		for (auto kv : mTextures) {
			delete kv.second;
		}
	}

	IVertexBuffer* OpenglResourceManager::CreateVertexBufferImpl() {
		IP_ENGINE_TRACE("OpenglResourceManager create vertex buffer");
		IVertexBuffer* newVertexBuffer = new OpenglVertexBuffer();
		mVertexBuffers.push_back(newVertexBuffer);
		return mVertexBuffers.back();
	}

	IIndexBuffer* OpenglResourceManager::CreateIndexBufferImpl() {
		IP_ENGINE_TRACE("OpenglResourceManager create index buffer");
		IIndexBuffer* newIndexBuffer = new OpenglIndexBuffer();
		mIndexBuffers.push_back(newIndexBuffer);
		return mIndexBuffers.back();
	}

	IShader* OpenglResourceManager::GetShaderFromSourceImpl(ShaderData shaderData) {
		if (mShaders.contains(shaderData.Name)) {
			IP_ENGINE_WARN("Shader name has been existed, source will be replaced");
			delete mShaders.at(shaderData.Name);
		}

		IP_ENGINE_TRACE("OpenglResourceManager inserting shader name: {}", shaderData.Name);
		IP_ENGINE_DEBUG("Vertex src: \n{}", shaderData.VertexSrc);
		IP_ENGINE_DEBUG("Fragment src: \n{}", shaderData.FragmentSrc);

		IShader* newShader = new OpenglShader(shaderData.VertexSrc, shaderData.FragmentSrc);
		mShaders.insert({ shaderData.Name, newShader });
		IP_ENGINE_TRACE("OpenglResourceManager getting shader name: {}", shaderData.Name);
		return mShaders.at(shaderData.Name);
	}

	IShader* OpenglResourceManager::GetShaderFromFileImpl(const char* file) {
		std::string fileName(file);
		if (mShaders.contains(fileName)) {
			IP_ENGINE_WARN("Shader name has been existed, source will be replaced");
			delete mShaders.at(fileName);
		}

		bool isVertexSrc = true;
		std::ifstream fs(file);
		if (!fs.is_open()) {
			IP_ENGINE_ERROR("File can't open! return null");
			return nullptr;
		}

		std::string line;
		std::getline(fs, line);
		if (line != "#type vertex") {
			IP_ENGINE_CRITICAL("Wrong file format! Header should be #type vertex, return null");
			fs.close();
			return nullptr;
		}
		
		// In C++ there is no String Builder sadly
		std::string vertexSrc;
		while (std::getline(fs,line)) {
			if (line == "#type fragment") {
				isVertexSrc = false;
				break;
			}

			vertexSrc.append(line);
			vertexSrc.append("\n");
		}

		if (isVertexSrc) {
			IP_ENGINE_CRITICAL("Wrong file format! Missing #type fragment, return null");
			fs.close();

		}

		std::string fragmentSrc;
		while (std::getline(fs, line)) {
			fragmentSrc.append(line);
			fragmentSrc.append("\n");
		}

		fs.close();
		IP_ENGINE_DEBUG("Vertex src: \n{}", vertexSrc);
		IP_ENGINE_DEBUG("Fragment src: \n{}", fragmentSrc);

		IP_ENGINE_TRACE("OpenglResourceManager created shader with name: {}", fileName);
		IShader* newShader = new OpenglShader(vertexSrc, fragmentSrc);
		mShaders.insert({ fileName, newShader });
		return newShader;
	}

	IShader* OpenglResourceManager::GetShaderFromNameImpl(const std::string& name) {
		if (mShaders.contains(name)) {
			IP_ENGINE_TRACE("OpenglResourceManager get shader with name: {}", name);
			return mShaders.at(name);
		}

		IP_ENGINE_TRACE("OpenglResourceManager not found name, return null");
		return nullptr;
	}

	ITexture* OpenglResourceManager::GetTextureFromFileImpl(const char* file) {
		std::string fileName(file);
		if (mTextures.contains(fileName)) {
			IP_ENGINE_WARN("OpenglResourceManager found texture with name: {}", fileName);
			return mTextures.at(fileName);
		}

		IP_ENGINE_TRACE("OpenglResourceManager created new texture with file: {}", fileName);
		ITexture* newTexture = new OpenglTexture(fileName);
		mTextures.insert({ fileName, newTexture });
		return newTexture;
	}
}