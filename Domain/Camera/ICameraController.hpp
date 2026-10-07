#pragma once
#include <Camera/ICamera.hpp>

namespace Domain::Camera
{
	class ICameraController
	{
	public:
		virtual ~ICameraController() = default;
		virtual void Update(float deltaTime) = 0;
		virtual ICamera& GetCamera() = 0;
	};
}