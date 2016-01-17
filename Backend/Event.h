#ifndef EVENT_BACKEND
#define EVENT_BACKEND

#include "Date.h"

#include <functional>

namespace bEnd
{
	class EventBase
	{
	public:
		virtual ~EventBase() {};

		const bool operator< (const EventBase& _event)const { return trigger < _event.trigger; }

		virtual void initiate()const = 0;

		Date trigger;
	};

	template <typename functionType = void()>
	class Event final : public EventBase
	{
	public:
		Event(const Date& trigger, const std::function<functionType>& action) : trigger(trigger), action(action) {}
		Event(const Event&) = default;
		Event(Event&&) = default;
		Event() = delete;
		~Event()override = default;

		Event& operator=(const Event&) = default;
		Event& operator=(Event&&) = default;

		void initiate()const override { action(); }
	private:
		const std::function<functionType> action;
	};
}
#endif