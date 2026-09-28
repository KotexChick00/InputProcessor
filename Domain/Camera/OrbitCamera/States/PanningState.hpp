#pragma once
#include <Camera/OrbitCamera/IOrbitControllerState.hpp>

namespace Domain::Camera::OrbitCamera::States
{
	class PanningState : public IOrbitControllerState
	{
	public:
		void OnEnter(OrbitCameraController& controller) override;
		void Update(OrbitCameraController& controller, float deltaTime) override;
		void OnExit(OrbitCameraController& controller) override;
	};
} // namespace Domain::Camera::OrbitCamera::States