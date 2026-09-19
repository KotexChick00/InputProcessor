#pragma once
#include <iostream>
#include <functional>
#include <Event/EventContext.hpp>

namespace CoreEngine::Event {
	template<typename T> using EventCallback = std::function<bool(const T&)>;

	class IEventAction {
	public:
		virtual ~IEventAction() = default;
		virtual void Handle(EventContext* eventContext) = 0;
	};

	template<typename T>
	class EventAction : public IEventAction {
	public:
		EventAction(const EventCallback<T>& callback) : mCallback(callback) { }

		void Handle(EventContext* eventContext) override {
			mCallback(*(T*)&eventContext);
		}

	private:
		EventCallback<T> mCallback;
	};
}