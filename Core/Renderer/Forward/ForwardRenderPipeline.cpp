#include <Renderer/Forward/ForwardRenderPipeline.hpp>
#include <Logger/Logger.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace CoreEngine::Renderer::Forward {
	using namespace CoreEngine::Logger;

	ForwardRenderPipeline::ForwardRenderPipeline(IRenderer* renderer)
		: mRenderer(renderer) {
		// Bản này CHƯA tạo offscreen framebuffer (sceneTarget/ping/pong).
		// Render thẳng ra default framebuffer (screen) để có bản chạy được trước.
		// IResourceManager::CreateColorFrameBuffer(...) đã sẵn sàng dùng khi cần,
		// xem chỗ bật trong EndFrame() bên dưới.
	}

	void ForwardRenderPipeline::BeginFrame() {
		mSubmissions.clear();
	}

	void ForwardRenderPipeline::Submit(const RenderSubmission& submission) {
		if (!submission.Renderable || !submission.Shader) {
			IP_ENGINE_WARN("ForwardRenderPipeline::Submit: renderable or shader is null, skip");
			return;
		}
		mSubmissions.push_back(submission);
	}

	void ForwardRenderPipeline::EndFrame(Domain::Camera::ICamera& camera) {
		// --- Geometry + Lighting Pass (gộp làm 1, đúng bản chất Forward) ---
		// Render thẳng ra screen ở bản này (chưa Bind() offscreen sceneTarget).
		mRenderer->GetRendererCommand()->ClearBuffers(ClearBufferMasks::Color | ClearBufferMasks::Depth);

		glm::mat4 viewProjection = camera.GetProjectionMatrix() * camera.GetViewMatrix();

		for (auto& submission : mSubmissions) {
			submission.Shader->Use();
			submission.Shader->SetUniformMatrix4fv("uViewProjection", glm::value_ptr(viewProjection));
			submission.Renderable->Draw(mRenderer, submission.Shader);
		}

		// NOTE: Post-Process Pass tạm thời KHÔNG chạy ở bản này.
		// Khi làm offscreen pass thật, cần:
		//   1. Tạo sceneTarget/ping/pong qua IResourceManager::CreateColorFrameBuffer()
		//      (gọi 1 lần trong constructor, lưu FrameBufferID).
		//   2. Bind sceneTarget TRƯỚC vòng for ở trên, Unbind SAU vòng for.
		//   3. Loop qua mPostProcessEffects, Apply(input, output), ping-pong input/output.
		//   4. Vẽ full-screen quad đọc buffer cuối cùng ra screen (chưa có quad mesh).
		if (!mPostProcessEffects.empty()) {
			IP_ENGINE_WARN(
				"ForwardRenderPipeline::EndFrame: {} post-process effect(s) registered "
				"but not yet applied (offscreen pass chưa implement)",
				mPostProcessEffects.size()
			);
		}
	}

	void ForwardRenderPipeline::OnResize(unsigned int width, unsigned int height) {
		ViewPortSize newSize{ width, height };
		for (auto* effect : mPostProcessEffects) {
			effect->OnResize(newSize);
		}
		// TODO: resize lại sceneTarget/ping/pong khi offscreen pass đã implement
	}

	void ForwardRenderPipeline::AddPostProcessEffect(IPostProcessEffect* effect) {
		mPostProcessEffects.push_back(effect);
	}
}