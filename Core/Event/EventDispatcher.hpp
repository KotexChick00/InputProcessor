#pragma once
#include <iostream>
#include <Event/EventAction.hpp>
#include <unordered_map>

namespace InputProcessor::Event {
	class EventDispatcher {
	public:
		EventDispatcher() = default;

		~EventDispatcher() {
			for (auto pair : mEventActionMappers) {
				for (IEventAction* eventAction : pair.second) delete eventAction;
			}
		}

		template<typename T>
		void AddEventListener(const EventCallback<T>& eventCallback) {
			const char* eventName = typeid(T).name();
			IEventAction* eventAction = new EventAction(eventCallback);
			mEventActionMappers[eventName].push_back(eventAction);
		}

		template<typename T>
		void Notify(const T& eventContext) {
			const char* eventName = typeid(T).name();
			if (mEventActionMappers.contains(eventName)) {
				std::vector<IEventAction*> eventActions = mEventActionMappers.at(eventName);
				for (IEventAction* eventAction : eventActions) 
					eventAction->Handle(eventContext);
			}
		}

	private:
		std::unordered_map<const char*, std::vector<IEventAction*>> mEventActionMappers;
	};
}