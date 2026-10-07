#pragma once
#include <Camera/OrbitCamera/IOrbitControllerState.hpp>

namespace Domain::Camera::OrbitCamera::States
{
	class OrbitingState : public IOrbitControllerState
	{
	public:
		void Update(OrbitCameraController& controller, float deltaTime) override;
	};
} // namespace Domain::Camera::OrbitCamera::States