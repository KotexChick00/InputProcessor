#include <Renderer/Opengl/OpenglTexture.hpp>
#include <Renderer/Opengl/OpenglConstantFactory.hpp>
#include <Renderer/Opengl/OpenglResourceManager.hpp>
#include <Logger/Logger.hpp>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace InputProcessor::Renderer::Resource::Opengl {
	using namespace InputProcessor::Logger;

	// Use OpenglConstantFactory for enum conversions

	// --- Constructor & Destructor ---

	OpenglTexture::OpenglTexture(GLuint textId)
		: mTextureIdx(textId)
	{
		// Apply default configuration
		Config(TextureConfiguration{});
	}

	OpenglTexture::~OpenglTexture() {
		if (mTextureIdx != 0) {
			IP_ENGINE_TRACE("Texture destroy with id: {}", mTextureIdx);
			OpenglResourceManager::GetInstance()->RemoveTexture(GetTextureId());
			glDeleteTextures(1, &mTextureIdx);
			mTextureIdx = 0;
		}
	}

	// --- Configuration & Binding ---

	void OpenglTexture::Config(const TextureConfiguration& config) {
		if (mTextureIdx == 0) {
			IP_ENGINE_WARN("Cannot configure an uninitialized texture (ID: 0).");
			return;
		}

		glBindTexture(GL_TEXTURE_2D, mTextureIdx);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenglConstantFactory::ToGLEnum(config.WrapSMethod));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenglConstantFactory::ToGLEnum(config.WrapTMethod));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OpenglConstantFactory::ToGLEnum(config.MinFilter));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OpenglConstantFactory::ToGLEnum(config.MagFilter));

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	TextureID OpenglTexture::GetTextureId() const {
		return static_cast<TextureID>(mTextureIdx);
	}

	GLuint OpenglTexture::GetOpenglId() const {
		return mTextureIdx;
	}

	OpenglTexture* OpenglTexture::FromFile(const std::string& file) {
		stbi_set_flip_vertically_on_load(true);

		int width = 0;
		int height = 0;
		int channels = 0;
		unsigned char* data = stbi_load(file.c_str(), &width, &height, &channels, 0);

		if (!data) {
			IP_ENGINE_ERROR("Failed to load texture from file path: '{}', return nullptr", file);
			return nullptr;
		}

		GLenum internalFormat = 0;
		GLenum dataFormat = 0;

		if (channels == 4) {
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3) {
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}
		else if (channels == 1) {
			internalFormat = GL_R8;
			dataFormat = GL_RED;
		}
		else {
			IP_ENGINE_WARN("Unsupported channel count ({}) for texture: '{}', return nullptr", channels, file);
			stbi_image_free(data);
			return nullptr;
		}

		GLuint textId;
		glGenTextures(1, &textId);
		glBindTexture(GL_TEXTURE_2D, textId);

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			internalFormat,
			width,
			height,
			0,
			dataFormat,
			GL_UNSIGNED_BYTE,
			data
		);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(data);

		IP_ENGINE_TRACE("Loaded texture '{}' [{}x{}, {} ch, ID: {}]", file, width, height, channels, textId);
	
		OpenglTexture* texture = new OpenglTexture(textId);
		OpenglResourceManager::GetInstance()->InsertTexture(texture);
		return texture;
	}
}
