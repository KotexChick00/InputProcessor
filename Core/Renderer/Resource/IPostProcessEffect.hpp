#pragma once
#include <pch.h>
#include <Renderer/Resource/IFrameBuffer.hpp>

namespace CoreEngine::Renderer {
	class CORE_API IPostProcessEffect {
	public:
		virtual ~IPostProcessEffect() = default;
		virtual void ApplyEffect(IFrameBuffer* inputFrameBuffer, IFrameBuffer* outputFrameBuffer) = 0;

		virtual void OnResize(const ViewPortSize& newSize) = 0;

		virtual bool IsEnabled() const = 0;
		virtual void SetEnabled(bool enabled) = 0;

		virtual const std::string& GetEffectName() const = 0;
	};
}