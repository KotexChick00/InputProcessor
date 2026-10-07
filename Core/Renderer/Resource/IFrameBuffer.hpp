#pragma once
#include <pch.h>
#include <Renderer/Resource/IShader.hpp>

namespace CoreEngine::Renderer {
	class CORE_API IFrameTextureAttachment {
	public:
		virtual ~IFrameTextureAttachment() = default;

		virtual void SetTexture(const std::string& samplerName, IShader* shader) = 0;
		virtual void SetTexture(const std::string& samplerName, unsigned int textureIdx, IShader* shader) = 0;
	};

	typedef unsigned int FrameBufferID;

	struct CORE_API ViewPortSize {
		unsigned int Width;
		unsigned int Height;
	};

	// Currently support Color Attachment and Depth Attachment
	class CORE_API IFrameBuffer {
	public:
		virtual ~IFrameBuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual IFrameTextureAttachment* GetFrameTextureAttachment() = 0;

		virtual ViewPortSize GetViewPortSize() = 0;

		virtual FrameBufferID GetFrameBufferId() const = 0;
	};
}