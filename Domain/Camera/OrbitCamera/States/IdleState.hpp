#pragma once

#include <Camera/OrbitCamera/IOrbitControllerState.hpp>

namespace Domain::Camera::OrbitCamera::States
{
	class IdleState : public IOrbitControllerState
	{
	public:
		void Update(OrbitCameraController& controller, float deltaTime) override;
	};
} // namespace Domain::Camera::OrbitCamera::States
