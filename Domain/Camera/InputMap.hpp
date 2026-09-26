#pragma once
#include <optional>
#include <unordered_map>
#include <Camera/InputBinding.hpp>
#include <Input/InputState.hpp>

namespace Domain::Camera
{
	template<typename TAction>
	class InputMap
	{
	public:
		void BindAction(TAction action, InputBinding input)
		{
			m_ActionBindings[action] = input;
		}
		void UnbindAction(TAction action)
		{
			m_ActionBindings.erase(action);
		}
		bool IsJustPressed(TAction action, const CoreEngine::Input::InputState& inputState) const
		{
			return QueryAction(action, inputState,
				[&](CoreEngine::Input::KeyboardKey key) { return inputState.KeyboardInput->CheckIsJustPressed(key); },
				[&](CoreEngine::Input::MouseButton button) { return inputState.MouseInput->CheckIsJustPressed(button); },
				[&](MouseScrollDirection scrollDir) {
					return IsScrolling(scrollDir, inputState);
				});
		}
		bool IsContinuousPressed(TAction action, const CoreEngine::Input::InputState& inputState) const
		{
			return QueryAction(action, inputState,
				[&](CoreEngine::Input::KeyboardKey key) { return inputState.KeyboardInput->CheckIsPressed(key); },
				[&](CoreEngine::Input::MouseButton button) { return inputState.MouseInput->CheckIsPressed(button); },
				[&](MouseScrollDirection scrollDir) {
					return IsScrolling(scrollDir, inputState);
				});
		}
		bool IsReleased(TAction action, const CoreEngine::Input::InputState& inputState) const
		{
			return QueryAction(action, inputState,
				[&](CoreEngine::Input::KeyboardKey key) { return inputState.KeyboardInput->CheckIsReleased(key); },
				[&](CoreEngine::Input::MouseButton button) { return inputState.MouseInput->CheckIsReleased(button); },
				[&](MouseScrollDirection scrollDir) {
					return !IsScrolling(scrollDir, inputState);
				});
		}
	private:
		static bool IsScrolling(MouseScrollDirection scrollDir, const CoreEngine::Input::InputState& inputState)
		{
			constexpr float kEpsilon = 0.0001f; // Độ lệch nhỏ để so sánh float
			float scrollY = inputState.MouseInput->GetScrollY();
			if (scrollY < kEpsilon && scrollY > -kEpsilon) return false; // Không có cuộn
			if (scrollDir == MouseScrollDirection::Up)
				return scrollY > 0.0f;
			else if (scrollDir == MouseScrollDirection::Down)
				return scrollY < 0.0f;
			return false;

		}
		template<typename KeyboardFunc, typename MouseFunc, typename ScrollFunc>
		bool QueryAction(TAction action, const CoreEngine::Input::InputState& inputState, KeyboardFunc keyboardFunc, MouseFunc mouseFunc, ScrollFunc scrollFunc) const
		{
			auto it = m_ActionBindings.find(action);
			if (it == m_ActionBindings.end())
				return false;
			const InputBinding& binding = it->second;
			if (std::holds_alternative<CoreEngine::Input::KeyboardKey>(binding))
			{
				CoreEngine::Input::KeyboardKey key = std::get<CoreEngine::Input::KeyboardKey>(binding);
				return keyboardFunc(key);
			}
			else if (std::holds_alternative<CoreEngine::Input::MouseButton>(binding))
			{
				CoreEngine::Input::MouseButton button = std::get<CoreEngine::Input::MouseButton>(binding);
				return mouseFunc(button);
			}
			else if (std::holds_alternative<MouseScrollDirection>(binding))
			{
				MouseScrollDirection scrollDir = std::get<MouseScrollDirection>(binding);
				return scrollFunc(scrollDir);
			}
			return false;
		}
		std::unordered_map<TAction, InputBinding> m_ActionBindings;
	};
}
