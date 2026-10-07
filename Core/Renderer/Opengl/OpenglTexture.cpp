#include <Renderer/Opengl/OpenglTexture.hpp>
#include <Renderer/Opengl/OpenglConstantFactory.hpp>
#include <Renderer/Opengl/OpenglResourceManager.hpp>
#include <Logger/Logger.hpp>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace CoreEngine::Renderer::Opengl {
	using namespace CoreEngine::Logger;

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
        int width = 0, height = 0, channels = 0;
        auto* pixels = stbi_load(file.c_str(), &width, &height, &channels, 4);
        if (!pixels) {
            IP_ENGINE_ERROR("Failed to load texture '{}'", file);
            return nullptr;
        }
        return Upload(pixels, width, height);
    }

    OpenglTexture* OpenglTexture::FromMemory(const unsigned char* data, unsigned int size) {
        if (!data || !size || size > static_cast<unsigned int>(std::numeric_limits<int>::max())) return nullptr;
        stbi_set_flip_vertically_on_load(true);
        int width = 0, height = 0, channels = 0;
        auto* pixels = stbi_load_from_memory(data, static_cast<int>(size), &width, &height, &channels, 4);
        if (!pixels) {
            IP_ENGINE_ERROR("Failed to decode embedded texture");
            return nullptr;
        }
        return Upload(pixels, width, height);
    }

    OpenglTexture* OpenglTexture::Upload(unsigned char* pixels, int width, int height) {
        GLuint id = 0;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(pixels);
        auto* texture = new OpenglTexture(id);
        OpenglResourceManager::GetInstance()->InsertTexture(texture);
        return texture;
    }
}
