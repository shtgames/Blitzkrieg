#ifndef TIME_SYSTEM_BACKEND
#define TIME_SYSTEM_BACKEND

#include "Date.h"

#include <memory>
#include <chrono>
#include <queue>
#include <functional>

using namespace std;

namespace bEnd
{
	class TimeSystem final
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
			Event(const Date& trigger, std::function<functionType>&& action) : trigger(trigger), action(std::move(action)) {}
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

	public:

		static const Date& getCurrentDate();
		static void        update();
		static void        pause();
		static void        increaseSpeed();
		static void        decreaseSpeed();
		template <typename eventFunctionType>
		static void        addEvent(const Event<eventFunctionType>& event);
		template <typename eventFunctionType>
		static void        addEvent(Event<eventFunctionType>&& event);
		static void        reset(const Date& date);

		static const chrono::high_resolution_clock gameTime;
	private:
				
		static void eventCheck();

		static priority_queue<unique_ptr<EventBase>>             events;
		static unsigned char                                     gameSpeed;
		static chrono::time_point<chrono::high_resolution_clock> timeOfLastUpdate;
		static float                                             updateIntervals[6];
		static Date                                              currentDate;
		static bool                                              paused;
	};
}
#endif