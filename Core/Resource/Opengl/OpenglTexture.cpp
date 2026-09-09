#include <Resource/Opengl/OpenglTexture.hpp>
#include <Logger/Logger.hpp>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace InputProcessor::Resource::Opengl {
    using namespace InputProcessor::Logger;

    // --- Static Helper Methods ---

    GLenum OpenglTexture::ToGLEnum(WrapMethod wrapMethod) {
        switch (wrapMethod) {
        case WrapMethod::Repeat:         return GL_REPEAT;
        case WrapMethod::MirroedRepeat:  return GL_MIRRORED_REPEAT;
        case WrapMethod::ClampToEdge:    return GL_CLAMP_TO_EDGE;
        case WrapMethod::ClampToBorder:  return GL_CLAMP_TO_BORDER;
        default:                         return GL_REPEAT;
        }
    }

    GLenum OpenglTexture::ToGLEnum(MinFilterMethod minFilterMethod) {
        switch (minFilterMethod) {
        case MinFilterMethod::Nearest:               return GL_NEAREST;
        case MinFilterMethod::BiLinear:              return GL_LINEAR;
        case MinFilterMethod::NearestMimapNearest:   return GL_NEAREST_MIPMAP_NEAREST;
        case MinFilterMethod::NearestMimapBiLinear:  return GL_NEAREST_MIPMAP_LINEAR;
        case MinFilterMethod::BiLinearMimapNearest:  return GL_LINEAR_MIPMAP_NEAREST;
        case MinFilterMethod::BiLinearMimapBiLinear: return GL_LINEAR_MIPMAP_LINEAR;
        default:                                     return GL_LINEAR_MIPMAP_LINEAR;
        }
    }

    GLenum OpenglTexture::ToGLEnum(MagFilterMethod magFilterMethod) {
        switch (magFilterMethod) {
        case MagFilterMethod::Nearest:  return GL_NEAREST;
        case MagFilterMethod::BiLinear: return GL_LINEAR;
        default:                        return GL_LINEAR;
        }
    }

    // --- Constructor & Destructor ---

    OpenglTexture::OpenglTexture(const std::string& file)
        : mTextureIdx(0)
    {
        stbi_set_flip_vertically_on_load(true);

        int width = 0;
        int height = 0;
        int channels = 0;
        unsigned char* data = stbi_load(file.c_str(), &width, &height, &channels, 0);

        if (!data) {
            IP_ENGINE_ERROR("Failed to load texture from file path: '{}'", file);
            return;
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
            IP_ENGINE_WARN("Unsupported channel count ({}) for texture: '{}'", channels, file);
            stbi_image_free(data);
            return;
        }

        glGenTextures(1, &mTextureIdx);
        glBindTexture(GL_TEXTURE_2D, mTextureIdx);

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

        // Áp dụng cấu hình mặc định (Repeat + Trilinear filtering)
        Config(TextureConfiguration{});

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);

        IP_ENGINE_TRACE("Loaded texture '{}' [{}x{}, {} ch, ID: {}]", file, width, height, channels, mTextureIdx);
    }

    OpenglTexture::~OpenglTexture() {
        if (mTextureIdx != 0) {
            IP_ENGINE_TRACE("Texture destroy with id: {}", mTextureIdx);
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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ToGLEnum(config.WrapSMethod));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ToGLEnum(config.WrapTMethod));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ToGLEnum(config.MinFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ToGLEnum(config.MagFilter));

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void OpenglTexture::SetTexture(const std::string& samplerName, IShader* shader) {
        SetTexture(samplerName, 0, shader);
    }

    void OpenglTexture::SetTexture(const std::string& samplerName, unsigned int textureIdx, IShader* shader) {
        if (mTextureIdx == 0) {
            IP_ENGINE_ERROR("Cannot bind invalid texture (ID: 0) to uniform '{}'", samplerName);
            return;
        }

        if (!shader) {
            IP_ENGINE_ERROR("Shader pointer is null when binding uniform '{}'", samplerName);
            return;
        }

        glActiveTexture(GL_TEXTURE0 + textureIdx);
        glBindTexture(GL_TEXTURE_2D, mTextureIdx);

        shader->Use();
        shader->SetUniform1i(samplerName, static_cast<int>(textureIdx));
    }

}