#include <Renderer/Opengl/OpenglConstantFactory.hpp>
#include <Logger/Logger.hpp>
#include <Renderer/Resource/ICubeMap.hpp>

namespace CoreEngine::Renderer::Opengl {
	using namespace CoreEngine::Logger;

	GLenum OpenglConstantFactory::ToGLRenderMode(VertexBufferRenderMode mode) {
		switch (mode) {
			case VertexBufferRenderMode::Static:
				return GL_STATIC_DRAW;
			case VertexBufferRenderMode::Stream:
				return GL_STREAM_DRAW;
			case VertexBufferRenderMode::Dynamic:
				return GL_DYNAMIC_DRAW;
			default:
				IP_ENGINE_WARN("Undefined mode, return static mode instead");
				return GL_STATIC_DRAW;
		}
	}

	GLenum OpenglConstantFactory::ToGLRenderMode(IndexBufferRenderMode mode) {
		switch (mode) {
		case IndexBufferRenderMode::Static:
			return GL_STATIC_DRAW;
		case IndexBufferRenderMode::Stream:
			return GL_STREAM_DRAW;
		case IndexBufferRenderMode::Dynamic:
			return GL_DYNAMIC_DRAW;
		default:
			IP_ENGINE_WARN("Undefined mode, return static mode instead");
			return GL_STATIC_DRAW;
		}
	}

	GLenum OpenglConstantFactory::ToGLType(VertexBufferDataType type) {
		switch (type) {
			case VertexBufferDataType::Float:
				return GL_FLOAT;
			case VertexBufferDataType::Int:
				return GL_INT;
			case VertexBufferDataType::UnsignedInt:
				return GL_UNSIGNED_INT;
			case VertexBufferDataType::Byte:
				return GL_BYTE;
			default:
				IP_ENGINE_WARN("Undefined data type, return float instead");
				return GL_FLOAT;
		}
	}

	GLenum OpenglConstantFactory::ToGLEnum(WrapMethod wrapMethod) {
		switch (wrapMethod) {
		case WrapMethod::Repeat:         return GL_REPEAT;
		case WrapMethod::MirroedRepeat:  return GL_MIRRORED_REPEAT;
		case WrapMethod::ClampToEdge:    return GL_CLAMP_TO_EDGE;
		case WrapMethod::ClampToBorder:  return GL_CLAMP_TO_BORDER;
		default:
			IP_ENGINE_WARN("Undefined WrapMethod, defaulting to GL_REPEAT");
			return GL_REPEAT;
		}
	}

	GLenum OpenglConstantFactory::ToGLEnum(MinFilterMethod minFilterMethod) {
		switch (minFilterMethod) {
		case MinFilterMethod::Nearest:               return GL_NEAREST;
		case MinFilterMethod::BiLinear:              return GL_LINEAR;
		case MinFilterMethod::NearestMimapNearest:   return GL_NEAREST_MIPMAP_NEAREST;
		case MinFilterMethod::NearestMimapBiLinear:  return GL_NEAREST_MIPMAP_LINEAR;
		case MinFilterMethod::BiLinearMimapNearest:  return GL_LINEAR_MIPMAP_NEAREST;
		case MinFilterMethod::BiLinearMimapBiLinear: return GL_LINEAR_MIPMAP_LINEAR;
		default:
			IP_ENGINE_WARN("Undefined MinFilterMethod, defaulting to GL_LINEAR_MIPMAP_LINEAR");
			return GL_LINEAR_MIPMAP_LINEAR;
		}
	}

	GLenum OpenglConstantFactory::ToGLEnum(MagFilterMethod magFilterMethod) {
		switch (magFilterMethod) {
		case MagFilterMethod::Nearest:  return GL_NEAREST;
		case MagFilterMethod::BiLinear: return GL_LINEAR;
		default:
			IP_ENGINE_WARN("Undefined MagFilterMethod, defaulting to GL_LINEAR");
			return GL_LINEAR;
		}
	}

	GLenum OpenglConstantFactory::ToGLWrap(CubeMapWrapMethod wrap) {
		switch (wrap) {
		case CubeMapWrapMethod::Repeat: return GL_REPEAT;
		case CubeMapWrapMethod::MirroedRepeat: return GL_MIRRORED_REPEAT;
		case CubeMapWrapMethod::ClampToEdge: return GL_CLAMP_TO_EDGE;
		case CubeMapWrapMethod::ClampToBorder: return GL_CLAMP_TO_BORDER;
		default:
			IP_ENGINE_WARN("Undefined CubeMapWrapMethod, defaulting to GL_CLAMP_TO_EDGE");
			return GL_CLAMP_TO_EDGE;
		}
	}

	GLenum OpenglConstantFactory::ToGLMinFilter(CubeMapMinFilterMethod minFilter) {
		switch (minFilter) {
		case CubeMapMinFilterMethod::Nearest: return GL_NEAREST;
		case CubeMapMinFilterMethod::BiLinear: return GL_LINEAR;
		case CubeMapMinFilterMethod::NearestMimapNearest: return GL_NEAREST_MIPMAP_NEAREST;
		case CubeMapMinFilterMethod::NearestMimapBiLinear: return GL_NEAREST_MIPMAP_LINEAR;
		case CubeMapMinFilterMethod::BiLinearMimapNearest: return GL_LINEAR_MIPMAP_NEAREST;
		case CubeMapMinFilterMethod::BiLinearMimapBiLinear: return GL_LINEAR_MIPMAP_LINEAR;
		default:
			IP_ENGINE_WARN("Undefined CubeMapMinFilterMethod, defaulting to GL_LINEAR");
			return GL_LINEAR;
		}
	}

	GLenum OpenglConstantFactory::ToGLMagFilter(CubeMapMagFilterMethod magFilter) {
		switch (magFilter) {
		case CubeMapMagFilterMethod::Nearest: return GL_NEAREST;
		case CubeMapMagFilterMethod::BiLinear: return GL_LINEAR;
		default:
			IP_ENGINE_WARN("Undefined CubeMapMagFilterMethod, defaulting to GL_LINEAR");
			return GL_LINEAR;
		}
	}
}
