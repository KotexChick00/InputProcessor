#pragma once
#include <pch.h>
#include <Event/EventContext.hpp>

namespace CoreEngine::Event {
	template<typename T> using EventCallback = std::function<bool(const T&)>;

	class IEventAction {
	public:
		virtual ~IEventAction() = default;
		virtual bool Handle(const EventContext& eventContext) = 0;
	};

	template<typename T>
	class EventAction : public IEventAction {
	public:
		EventAction(const EventCallback<T>& callback) : mCallback(callback) { }

		bool Handle(const EventContext& eventContext) override {
			return mCallback(*(T*)&eventContext);
		}

	private:
		EventCallback<T> mCallback;
	};
}