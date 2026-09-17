#include <Renderer/Opengl/OpenglFrameBuffer.hpp>
#include <Renderer/Opengl/OpenglFrameBufferManager.hpp>
#include <Logger/Logger.hpp>
#include <glad/glad.h>

namespace InputProcessor::Renderer::Resource::Opengl {
	using namespace InputProcessor::Logger;

	// --- OpenglFrameTextureAttachment ---
	OpenglFrameTextureAttachment::OpenglFrameTextureAttachment(GLuint texId, GLenum target)
		: mTexId(texId), mTarget(target) {}

	OpenglFrameTextureAttachment::~OpenglFrameTextureAttachment() {
		// Do not delete texture here; ownership belongs to framebuffer
		mTexId = 0;
	}

	void OpenglFrameTextureAttachment::SetTexture(const std::string& samplerName, IShader* shader) {
		SetTexture(samplerName, 0, shader);
	}

	void OpenglFrameTextureAttachment::SetTexture(const std::string& samplerName, unsigned int textureIdx, IShader* shader) {
		if (mTexId == 0) {
			IP_ENGINE_ERROR("FrameTextureAttachment: invalid texture id (0) for sampler '{}'", samplerName);
			return;
		}
		if (!shader) {
			IP_ENGINE_ERROR("FrameTextureAttachment: shader is null when binding sampler '{}'", samplerName);
			return;
		}

		glActiveTexture(GL_TEXTURE0 + textureIdx);
		glBindTexture(mTarget, mTexId);
		shader->Use();
		shader->SetUniform1i(samplerName, static_cast<int>(textureIdx));
	}

	// --- OpenglColorFrameBuffer ---
	OpenglColorFrameBuffer::OpenglColorFrameBuffer(unsigned int width, unsigned int height)
		: mWidth(width), mHeight(height) {
		glGenFramebuffers(1, &mFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, mFbo);

		// Create color texture
		glGenTextures(1, &mTexture);
		glBindTexture(GL_TEXTURE_2D, mTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);

		// Create renderbuffer for depth + stencil
		glGenRenderbuffers(1, &mRbo);
		glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			IP_ENGINE_ERROR("Failed to create complete framebuffer (color)");
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		mAttachment = new OpenglFrameTextureAttachment(mTexture, GL_TEXTURE_2D);
		IP_ENGINE_TRACE("Created color framebuffer (FBO: {}, Tex: {}, RBO: {})", mFbo, mTexture, mRbo);
	}

	OpenglColorFrameBuffer::~OpenglColorFrameBuffer() {
		OpenglFrameBufferManager::GetInstance()->DeleteFrameBuffer(GetFrameBufferId());
		if (mAttachment) { delete mAttachment; mAttachment = nullptr; }
		if (mTexture != 0) { glDeleteTextures(1, &mTexture); mTexture = 0; }
		if (mRbo != 0) { glDeleteRenderbuffers(1, &mRbo); mRbo = 0; }
		if (mFbo != 0) { glDeleteFramebuffers(1, &mFbo); mFbo = 0; }
	}

	void OpenglColorFrameBuffer::Bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
	}

	void OpenglColorFrameBuffer::Unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	IFrameTextureAttachment* OpenglColorFrameBuffer::GetFrameTextureAttachment() {
		return mAttachment;
	}

	FrameBufferID OpenglColorFrameBuffer::GetFrameBufferId() const {
		return static_cast<FrameBufferID>(mFbo);
	}

	ViewPortSize OpenglColorFrameBuffer::GetViewPortSize() {
		return { mWidth, mHeight };
	}

	OpenglColorFrameBuffer* OpenglColorFrameBuffer::Create(FrameBufferCreateInfo createInfo) {
		OpenglColorFrameBuffer* newColorFrameBuffer = new OpenglColorFrameBuffer(createInfo.Width, createInfo.Height);
		OpenglFrameBufferManager::GetInstance()->InsertFrameBuffer(newColorFrameBuffer);
		return newColorFrameBuffer;
	}

	// --- OpenglDepthFrameBuffer ---
	OpenglDepthFrameBuffer::OpenglDepthFrameBuffer(unsigned int width, unsigned int height)
		: mWidth(width), mHeight(height) {
		glGenFramebuffers(1, &mFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, mFbo);

		// Create depth texture
		glGenTextures(1, &mDepthTexture);
		glBindTexture(GL_TEXTURE_2D, mDepthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthTexture, 0);

		// Create stencil renderbuffer
		glGenRenderbuffers(1, &mStencilRbo);
		glBindRenderbuffer(GL_RENDERBUFFER, mStencilRbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mStencilRbo);

		// Since we have no color attachment, tell GL we won't render color
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			IP_ENGINE_ERROR("Failed to create complete framebuffer (depth)");
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		mAttachment = new OpenglFrameTextureAttachment(mDepthTexture, GL_TEXTURE_2D);
		IP_ENGINE_TRACE("Created depth framebuffer (FBO: {}, DepthTex: {}, StencilRBO: {})", mFbo, mDepthTexture, mStencilRbo);
	}

	OpenglDepthFrameBuffer::~OpenglDepthFrameBuffer() {
		OpenglFrameBufferManager::GetInstance()->DeleteFrameBuffer(GetFrameBufferId());
		if (mAttachment) { delete mAttachment; mAttachment = nullptr; }
		if (mDepthTexture != 0) { glDeleteTextures(1, &mDepthTexture); mDepthTexture = 0; }
		if (mStencilRbo != 0) { glDeleteRenderbuffers(1, &mStencilRbo); mStencilRbo = 0; }
		if (mFbo != 0) { glDeleteFramebuffers(1, &mFbo); mFbo = 0; }
	}

	void OpenglDepthFrameBuffer::Bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
	}

	void OpenglDepthFrameBuffer::Unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	IFrameTextureAttachment* OpenglDepthFrameBuffer::GetFrameTextureAttachment() {
		return mAttachment;
	}

	FrameBufferID OpenglDepthFrameBuffer::GetFrameBufferId() const {
		return static_cast<FrameBufferID>(mFbo);
	}

	ViewPortSize OpenglDepthFrameBuffer::GetViewPortSize() {
		return { mWidth, mHeight };
	}

	OpenglDepthFrameBuffer* OpenglDepthFrameBuffer::Create(FrameBufferCreateInfo createInfo) {
		OpenglDepthFrameBuffer* newDepthFrameBuffer = new OpenglDepthFrameBuffer(createInfo.Width, createInfo.Height);
		OpenglFrameBufferManager::GetInstance()->InsertFrameBuffer(newDepthFrameBuffer);
		return newDepthFrameBuffer;
	}
}
