#pragma once
#include <glad/glad.h>
#include <Renderer/Resource/IVertexBuffer.hpp>
#include <Renderer/Resource/IIndexBuffer.hpp>
#include <Renderer/RendererPrimitives.hpp>
#include <Renderer/Resource/ITexture.hpp>
#include <Renderer/Resource/ICubeMap.hpp>

namespace CoreEngine::Renderer::Opengl {
	class OpenglConstantFactory {
	public:
		static GLenum ToGLRenderMode(VertexBufferRenderMode mode);
		static GLenum ToGLRenderMode(IndexBufferRenderMode mode);
		static GLenum ToGLType(VertexBufferDataType type);
		static GLenum ToGLEnum(WrapMethod wrapMethod);
		static GLenum ToGLEnum(MinFilterMethod minFilterMethod);
		static GLenum ToGLEnum(MagFilterMethod magFilterMethod);
		static GLenum ToGLWrap(CubeMapWrapMethod wrap);
		static GLenum ToGLMinFilter(CubeMapMinFilterMethod minFilter);
		static GLenum ToGLMagFilter(CubeMapMagFilterMethod magFilter);
	};
}
