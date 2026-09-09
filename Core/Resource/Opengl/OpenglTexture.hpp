#pragma once
#include <Resource/ITexture.hpp>
#ifdef _USE_GLAD_OPENGL
#include <glad/glad.h>
#endif

namespace InputProcessor::Resource::Opengl {
	class OpenglTexture : public ITexture {
	public:
		OpenglTexture(const std::string& file);
		~OpenglTexture();

		void Config(const TextureConfiguration& config) override;

		/*
		* Texture idx will automatically be 0
		*/
		void SetTexture(const std::string& samplerName, IShader* shader) override;

		/*
		* Choose texture idx
		*/
		void SetTexture(const std::string& samplerName, unsigned int textureIdx, IShader* shader) override;

	private:
		unsigned int mTextureIdx;
		static GLenum ToGLEnum(WrapMethod wrapMethod);
		static GLenum ToGLEnum(MinFilterMethod minFilterMethod);
		static GLenum ToGLEnum(MagFilterMethod magFilterMethod);
	};
}