#pragma once
#include <Renderer/Resource/ITexture.hpp>
#include <glad/glad.h>

namespace InputProcessor::Renderer::Resource::Opengl {
	class OpenglTexture : public ITexture {
	public:
		OpenglTexture(GLuint textId);
		~OpenglTexture();

		// Return the native OpenGL texture id
		GLuint GetOpenglId() const;

		void Config(const TextureConfiguration& config) override;

		TextureID GetTextureId() const override;

	private:
		GLuint mTextureIdx;

	public:
		static OpenglTexture* FromFile(const std::string& file);
	};
}
