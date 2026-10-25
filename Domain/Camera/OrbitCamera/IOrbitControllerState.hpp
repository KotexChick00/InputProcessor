#pragma once

#include <Camera/OrbitCamera/OrbitCamera.hpp>

namespace Domain::Camera::OrbitCamera
{
	class OrbitCameraController;

	class IOrbitControllerState
	{
	public:
		virtual ~IOrbitControllerState() = default;
		virtual void OnEnter(OrbitCameraController& controller) {};
		virtual void OnExit(OrbitCameraController& controller) {};
		virtual void Update(OrbitCameraController& controller, float deltaTime) {};
	};
} // namespace Domain::Camera::OrbitCamera