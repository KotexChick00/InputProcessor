#pragma once
#include <iostream>
#include <glm/glm.hpp>

namespace InputProcessor::Domain {
	struct Transform {
		glm::vec3 Position;
		glm::vec3 Scale;
		glm::vec3 Rotation; // Euler Angle of Yaw - Pitch - Roll
	};

	class IRenderable {
	public:
		virtual ~IRenderable() = default;

		virtual void Render(Transform transform) = 0;
	};

	class IRenderableDecorator : public IRenderable {
	public:
		IRenderableDecorator(std::shared_ptr<IRenderable*> renderable) : mRenderable(renderable) {}
		virtual ~IRenderableDecorator() = default;

		virtual void Render(Transform transform) = 0;
	protected:
		std::shared_ptr<IRenderable*> mRenderable;
	};
}