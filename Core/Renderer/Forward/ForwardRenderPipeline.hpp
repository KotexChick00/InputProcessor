#pragma once
#include <pch.h>
#include <Renderer/IRenderPipeline.hpp>
#include <Renderer/IRenderer.hpp>
#include <Renderer/Resource/IPostProcessEffect.hpp>

namespace CoreEngine::Renderer::Forward {

	class CORE_API ForwardRenderPipeline : public IRenderPipeline {
	public:
		explicit ForwardRenderPipeline(IRenderer* renderer);
		~ForwardRenderPipeline() override = default;

		void BeginFrame() override;
		void Submit(const RenderSubmission& submission) override;
		void EndFrame(Domain::Camera::ICamera& camera) override;
		void OnResize(unsigned int width, unsigned int height) override;

		// Đã nhận effect, nhưng CHƯA được áp dụng trong EndFrame() ở bản này
		// (xem ghi chú NOTE trong .cpp) — chờ OpenglFullScreenQuad xong mới bật.
		void AddPostProcessEffect(IPostProcessEffect* effect);

	private:
		IRenderer* mRenderer;
		std::vector<RenderSubmission> mSubmissions;
		std::vector<IPostProcessEffect*> mPostProcessEffects;
	};
}