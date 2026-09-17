#pragma once
#include <iostream>
#include <Renderer/Resource/IShader.hpp>

namespace InputProcessor::Renderer::Resource {
	class IFrameTextureAttachment {
	public:
		virtual ~IFrameTextureAttachment() = default;

		virtual void SetTexture(const std::string& samplerName, IShader* shader) = 0;
		virtual void SetTexture(const std::string& samplerName, unsigned int textureIdx, IShader* shader) = 0;
	};

	typedef unsigned int FrameBufferID;

	// Currently support Color Attachment and Depth Attachment
	class IFrameBuffer {
	public:
		virtual ~IFrameBuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual IFrameTextureAttachment* GetFrameTextureAttachment() = 0;

		virtual FrameBufferID GetFrameBufferId() const = 0;
	};
}