#pragma once
#include <pch.h>
#include <Camera/ICamera.hpp>
#include <Renderer/Resource/IRenderable.hpp>
#include <Renderer/Resource/IShader.hpp>

namespace CoreEngine::Renderer {

	// Không giữ camera ở đây — cả frame chỉ có 1 camera active, truyền ở EndFrame().
	struct CORE_API RenderSubmission {
		IRenderable* Renderable = nullptr;
		IShader* Shader = nullptr;
	};

	class CORE_API IRenderPipeline {
	public:
		virtual ~IRenderPipeline() = default;

		virtual void BeginFrame() = 0;
		virtual void Submit(const RenderSubmission& submission) = 0;
		virtual void EndFrame(Domain::Camera::ICamera& camera) = 0;

		virtual void OnResize(unsigned int width, unsigned int height) = 0;
	};
}