#include <Renderer/Opengl/OpenglRenderer.hpp>
#include <Renderer/Opengl/OpenglResourceManager.hpp>
#include <Logger/Logger.hpp>
#include <glad/glad.h>

namespace CoreEngine::Renderer::Opengl {
	using namespace CoreEngine::Logger;

	OpenglRenderer* OpenglRenderer::GetInstance() {
		if (sInstance == nullptr) sInstance = new OpenglRenderer();
		return sInstance;
	}

	void OpenglRenderer::Free() {
		if (sInstance != nullptr) {
			delete sInstance;
			sInstance = nullptr;
		}
	}

	OpenglRenderer::OpenglRenderer() { }

	OpenglRenderer::~OpenglRenderer() {
		OpenglRendererCommand::Free();
		OpenglResourceManager::Free();
		OpenglFrameBufferManager::Free();
	}

	void OpenglRenderer::Config(RendererConfiguration configuration) {
		mConfiguration = configuration;

		// Clear color
		glClearColor(
			mConfiguration.ClearBufferColor.Red,
			mConfiguration.ClearBufferColor.Green,
			mConfiguration.ClearBufferColor.Blue,
			mConfiguration.ClearBufferColor.Alpha
		);

		// Viewport
		glViewport(
			mConfiguration.ViewPortOptions.X,
			mConfiguration.ViewPortOptions.Y,
			mConfiguration.ViewPortOptions.Width,
			mConfiguration.ViewPortOptions.Height
		);

		// Depth
		if (mConfiguration.DepthOptions.Enabled) {
			glEnable(GL_DEPTH_TEST);
			// depth func
			switch (mConfiguration.DepthOptions.PassFunc) {
			case DepthPassFunc::Always: glDepthFunc(GL_ALWAYS); break;
			case DepthPassFunc::Never: glDepthFunc(GL_NEVER); break;
			case DepthPassFunc::Less: glDepthFunc(GL_LESS); break;
			case DepthPassFunc::Equal: glDepthFunc(GL_EQUAL); break;
			case DepthPassFunc::LessEqual: glDepthFunc(GL_LEQUAL); break;
			case DepthPassFunc::Greater: glDepthFunc(GL_GREATER); break;
			case DepthPassFunc::NotEqual: glDepthFunc(GL_NOTEQUAL); break;
			case DepthPassFunc::GreaterEqual: glDepthFunc(GL_GEQUAL); break;
			}

			if (mConfiguration.DepthOptions.Operation == DepthOperation::ReadOnly) {
				glDepthMask(GL_FALSE);
			} else {
				glDepthMask(GL_TRUE);
			}
		} else {
			glDisable(GL_DEPTH_TEST);
		}

		// Cull face
		if (mConfiguration.CullFaceOptions.Enabled) {
			glEnable(GL_CULL_FACE);
			switch (mConfiguration.CullFaceOptions.CullFaceMode) {
			case CullFaceMode::Front: glCullFace(GL_FRONT); break;
			case CullFaceMode::Back: glCullFace(GL_BACK); break;
			case CullFaceMode::FrontAndBack: glCullFace(GL_FRONT_AND_BACK); break;
			}

			switch (mConfiguration.CullFaceOptions.CullFaceDirection) {
			case CullFaceDirection::ClockWise: glFrontFace(GL_CW); break;
			case CullFaceDirection::CounterClockWise: glFrontFace(GL_CCW); break;
			}
		} else {
			glDisable(GL_CULL_FACE);
		}

		// Blending
		if (mConfiguration.BlendOptions.Enabled) {
			glEnable(GL_BLEND);
			// Use simple premultiplied-like default: src alpha over dest
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		} else {
			glDisable(GL_BLEND);
		}

		// Stencil — leave default behavior unless needed; user can configure later
		if (mConfiguration.StencilOptions.Enabled) {
			glEnable(GL_STENCIL_TEST);
			// We won't map actions here; leave defaults
		} else {
			glDisable(GL_STENCIL_TEST);
		}

		IP_ENGINE_TRACE("OpenglRenderer configured");
	}

	IRendererCommand* OpenglRenderer::GetRendererCommand() {
		return OpenglRendererCommand::GetInstance();
	}

	IResourceManager* OpenglRenderer::GetResourceManager() {
		return OpenglResourceManager::GetInstance();
	}

	IFrameBufferManager* OpenglRenderer::GetFrameBufferManager() {
		return OpenglFrameBufferManager::GetInstance();
	}

	RendererConfiguration OpenglRenderer::GetConfig() {
		return mConfiguration;
	}
}
