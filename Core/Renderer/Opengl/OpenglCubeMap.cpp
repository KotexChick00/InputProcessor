#include <Renderer/Opengl/OpenglCubeMap.hpp>
#include <Renderer/Opengl/OpenglResourceManager.hpp>
#include <Renderer/Opengl/OpenglConstantFactory.hpp>
#include <Logger/Logger.hpp>
#include <glad/glad.h>
#include <array>

#include <stb_image.h>

namespace InputProcessor::Renderer::Resource::Opengl {
	using namespace InputProcessor::Logger;

	// --- Constructor / Destructor ---
	OpenglCubeMap::OpenglCubeMap(GLuint cubemap)
		: cubemapId(cubemap) {
	}

	OpenglCubeMap::~OpenglCubeMap() {
		if (cubemapId != 0) {
			IP_ENGINE_TRACE("OpenglCubeMap destroy with id: {}", cubemapId);
			OpenglResourceManager::GetInstance()->RemoveCubeMap(GetCubeMapId());
			glDeleteTextures(1, &cubemapId);
			cubemapId = 0;
		}
	}

	void OpenglCubeMap::Config(const CubeMapConfiguration& configuration) {
		if (cubemapId == 0) {
			IP_ENGINE_WARN("Cannot configure invalid cubemap (ID: 0)");
			return;
		}

		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapId);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, OpenglConstantFactory::ToGLMinFilter(configuration.MinFilterMethod));
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, OpenglConstantFactory::ToGLMagFilter(configuration.MagFilterMethod));
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, OpenglConstantFactory::ToGLWrap(configuration.WrapSMethod));
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, OpenglConstantFactory::ToGLWrap(configuration.WrapTMethod));
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, OpenglConstantFactory::ToGLWrap(configuration.WrapRMethod));

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	void OpenglCubeMap::SetCubeMap(const std::string& samplerName, IShader* shader) {
		SetCubeMap(samplerName, 0, shader);
	}

	void OpenglCubeMap::SetCubeMap(const std::string& samplerName, unsigned int textureIdx, IShader* shader) {
		if (cubemapId == 0) {
			IP_ENGINE_ERROR("Cannot bind invalid cubemap (ID: 0) to uniform '{}'", samplerName);
			return;
		}

		if (!shader) {
			IP_ENGINE_ERROR("Shader pointer is null when binding cubemap uniform '{}'", samplerName);
			return;
		}

		glActiveTexture(GL_TEXTURE0 + textureIdx);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapId);

		shader->Use();
		shader->SetUniform1i(samplerName, static_cast<int>(textureIdx));
	}

	CubeMapID OpenglCubeMap::GetCubeMapId() const {
		return static_cast<CubeMapID>(cubemapId);
	}

	OpenglCubeMap* OpenglCubeMap::FromFiles(CubeMapFiles cubemapFiles) {
		// Faces order: +X (right), -X (left), +Y (top), -Y (bottom), +Z (front), -Z (back)
		const std::array<std::pair<GLenum, std::string>, 6> faces = {
			std::make_pair(GL_TEXTURE_CUBE_MAP_POSITIVE_X, cubemapFiles.Right),
			std::make_pair(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, cubemapFiles.Left),
			std::make_pair(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, cubemapFiles.Top),
			std::make_pair(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, cubemapFiles.Bottom),
			std::make_pair(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, cubemapFiles.Front),
			std::make_pair(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, cubemapFiles.Back)
		};

		stbi_set_flip_vertically_on_load(false);

		GLuint texId = 0;
		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texId);

		for (const auto& face : faces) {
			const GLenum target = face.first;
			const std::string& file = face.second;

			int width = 0, height = 0, channels = 0;
			unsigned char* data = stbi_load(file.c_str(), &width, &height, &channels, 0);
			if (!data) {
				IP_ENGINE_ERROR("Failed to load cubemap face '{}' from path: '{}'", target, file);
				// cleanup
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
				if (texId != 0) glDeleteTextures(1, &texId);
				return nullptr;
			}

			GLenum internalFormat = 0;
			GLenum dataFormat = 0;
			if (channels == 4) { internalFormat = GL_RGBA8; dataFormat = GL_RGBA; }
			else if (channels == 3) { internalFormat = GL_RGB8; dataFormat = GL_RGB; }
			else if (channels == 1) { internalFormat = GL_R8; dataFormat = GL_RED; }
			else {
				IP_ENGINE_WARN("Unsupported channel count ({}) for cubemap face: '{}'", channels, file);
				stbi_image_free(data);
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
				if (texId != 0) glDeleteTextures(1, &texId);
				return nullptr;
			}

			glTexImage2D(target, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}

		// Set sensible defaults
		CubeMapConfiguration defaultConfig;
		defaultConfig.MagFilterMethod = CubeMapMagFilterMethod::BiLinear;
		defaultConfig.MinFilterMethod = CubeMapMinFilterMethod::BiLinearMimapBiLinear;
		defaultConfig.WrapSMethod = CubeMapWrapMethod::ClampToEdge;
		defaultConfig.WrapTMethod = CubeMapWrapMethod::ClampToEdge;
		defaultConfig.WrapRMethod = CubeMapWrapMethod::ClampToEdge;

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, OpenglConstantFactory::ToGLMinFilter(defaultConfig.MinFilterMethod));
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, OpenglConstantFactory::ToGLMagFilter(defaultConfig.MagFilterMethod));
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, OpenglConstantFactory::ToGLWrap(defaultConfig.WrapSMethod));
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, OpenglConstantFactory::ToGLWrap(defaultConfig.WrapTMethod));
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, OpenglConstantFactory::ToGLWrap(defaultConfig.WrapRMethod));

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		IP_ENGINE_TRACE("Loaded cubemap (ID: {})", texId);
		OpenglCubeMap* cubeMap = new OpenglCubeMap(texId);
		OpenglResourceManager::GetInstance()->InsertCubeMap(cubeMap);
		return cubeMap;
	}
}
