#pragma once
#include <Renderer/Resource/IShader.hpp>
#include <pch.h>

namespace CoreEngine::Renderer {
	enum class CORE_API UniformBufferRenderMode {
		Static = 0,
		Stream = 1,
		Dynamic = 2,
	};

	typedef unsigned int UniformBufferID;
	
	class CORE_API IUniformBuffer {
	public:
		virtual ~IUniformBuffer() = default;

		virtual void Allocate(const size_t& memorySize) = 0;
		virtual void SetData(void* data, size_t size, UniformBufferRenderMode renderMode = UniformBufferRenderMode::Static) = 0;
		virtual void SetData(void* data, size_t size, size_t offset, UniformBufferRenderMode renderMode = UniformBufferRenderMode::Static) = 0;
		
		virtual void BindLocation(unsigned int bindingPointIdx) = 0;

		virtual UniformBufferID GetUniformId() const = 0;
	};
}