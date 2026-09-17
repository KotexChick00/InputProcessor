#pragma once
#include <Renderer/Resource/IFrameBuffer.hpp>
#include <glad/glad.h>

namespace InputProcessor::Renderer::Resource::Opengl {
	class OpenglFrameTextureAttachment : public IFrameTextureAttachment {
	public:
		OpenglFrameTextureAttachment(GLuint texId, GLenum target = GL_TEXTURE_2D);
		~OpenglFrameTextureAttachment();

		void SetTexture(const std::string& samplerName, IShader* shader) override;
		void SetTexture(const std::string& samplerName, unsigned int textureIdx, IShader* shader) override;

	private:
		GLuint mTexId{0};
		GLenum mTarget{GL_TEXTURE_2D};
	};

	struct FrameBufferCreateInfo {
		unsigned int Width;
		unsigned int Height;
	};

	class OpenglColorFrameBuffer : public IFrameBuffer {
	public:
		~OpenglColorFrameBuffer() override;

		void Bind() override;
		void Unbind() override;

		IFrameTextureAttachment* GetFrameTextureAttachment() override;
		FrameBufferID GetFrameBufferId() const override;
		ViewPortSize GetViewPortSize() override;

	private:
		OpenglColorFrameBuffer(unsigned int width, unsigned int height);

		GLuint mFbo{0};
		GLuint mTexture{0};
		GLuint mRbo{0};
		unsigned int mWidth{0};
		unsigned int mHeight{0};
		OpenglFrameTextureAttachment* mAttachment{nullptr};

	public:
		static OpenglColorFrameBuffer* Create(FrameBufferCreateInfo createInfo);
	};

	class OpenglDepthFrameBuffer : public IFrameBuffer {
	public:
		~OpenglDepthFrameBuffer() override;

		void Bind() override;
		void Unbind() override;

		IFrameTextureAttachment* GetFrameTextureAttachment() override;
		FrameBufferID GetFrameBufferId() const override;
		ViewPortSize GetViewPortSize() override;

	private:
		OpenglDepthFrameBuffer(unsigned int width, unsigned int height);

		GLuint mFbo{0};
		GLuint mDepthTexture{0};
		GLuint mStencilRbo{0};
		unsigned int mWidth{0};
		unsigned int mHeight{0};
		OpenglFrameTextureAttachment* mAttachment{nullptr};

	public:
		static OpenglDepthFrameBuffer* Create(FrameBufferCreateInfo createInfo);
	};
}
