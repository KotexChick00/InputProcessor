#pragma once

#include <Camera/InputMap.hpp>
#include <Input/MouseInput.hpp>

namespace Domain::Camera::OrbitCamera
{
	enum class OrbitCameraAction
	{
		Orbit, // Rotate the camera around the target point
		Pan, // Move the camera parallel to the view plane
		Dolly, // Move the camera forward/backward along the view direction
		ZoomIn, // Move the camera closer to the target point
		ZoomOut, // Move the camera further from the target point
		Reset, // Reset the camera to its initial position and orientation
		OrbitLeft, OrbitRight, OrbitUp, OrbitDown,
		PanLeft, PanRight, PanUp, PanDown,
		DollyIn, DollyOut,
	};

	inline InputMap<OrbitCameraAction> CreateDefaultOrbitCameraInputMap()
	{

		InputMap<OrbitCameraAction> inputMap;
		inputMap.BindAction(OrbitCameraAction::Orbit, CoreEngine::Input::MouseButton::Button1);

		inputMap.BindAction(OrbitCameraAction::Pan, CoreEngine::Input::MouseButton::Button2);

		inputMap.BindAction(OrbitCameraAction::Dolly, CoreEngine::Input::MouseButton::Button3);

		inputMap.BindAction(OrbitCameraAction::ZoomIn, MouseScrollDirection::Up);

		inputMap.BindAction(OrbitCameraAction::ZoomOut, MouseScrollDirection::Down);

		inputMap.BindAction(OrbitCameraAction::Reset, CoreEngine::Input::KeyboardKey::R);
		using CoreEngine::Input::KeyboardKey;
		inputMap.BindAction(OrbitCameraAction::OrbitLeft, KeyboardKey::Left);
		inputMap.BindAction(OrbitCameraAction::OrbitRight, KeyboardKey::Right);
		inputMap.BindAction(OrbitCameraAction::OrbitUp, KeyboardKey::Up);
		inputMap.BindAction(OrbitCameraAction::OrbitDown, KeyboardKey::Down);
		inputMap.BindAction(OrbitCameraAction::PanLeft, KeyboardKey::A);
		inputMap.BindAction(OrbitCameraAction::PanRight, KeyboardKey::D);
		inputMap.BindAction(OrbitCameraAction::PanUp, KeyboardKey::W);
		inputMap.BindAction(OrbitCameraAction::PanDown, KeyboardKey::S);
		inputMap.BindAction(OrbitCameraAction::DollyIn, KeyboardKey::PageUp);
		inputMap.BindAction(OrbitCameraAction::DollyOut, KeyboardKey::PageDown);

		return inputMap;
	}
}
