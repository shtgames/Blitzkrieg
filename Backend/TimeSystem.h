#ifndef TIME_SYSTEM_BACKEND
#define TIME_SYSTEM_BACKEND

#include "Date.h"

#include <memory>
#include <chrono>
#include <queue>
#include <functional>
#include <mutex>
#include <atomic>
#include <vector>

using namespace std;

namespace bEnd
{
	typedef std::chrono::time_point<std::chrono::high_resolution_clock>(TimePoint);

	class Event final
	{
	public:
		Event(const Date& trigger, const std::function<void()>& action) : trigger(trigger), action(action) {}
		Event(const Date& trigger, std::function<void()>&& action) : trigger(trigger), action(std::move(action)) {}
		Event(const Event&) = default;
		Event(Event&&) = default;
		Event() = delete;
		~Event() = default;

		void initiate()const { action(); }
		const bool operator< (const Event& event)const { return trigger < event.trigger; }

		const Date trigger;

	private:
		const std::function<void()> action;
	};

	class TimeSystem final
	{
	public:
		static void        update();
		static void        pause();
		static void        resume();
		static void        increaseSpeed();
		static void        decreaseSpeed();
		static void        addEvent(const Event& event);
		static void        addEvent(Event&& event);
		static void        reset(const Date& date);

		static const bool isPaused();
		static const unsigned char getSpeed();
		static const Date& getCurrentDate();

		static const chrono::high_resolution_clock gameTime;

	private:
		typedef std::chrono::duration<float>(Duration);

		static void eventCheck();

		static priority_queue<unique_ptr<Event>> events;
		static std::atomic<unsigned char>        gameSpeed;
		static TimePoint                         timeOfLastUpdate;
		static const std::vector<float>          updateIntervals;
		static Date                              currentDate;
		static std::atomic<bool>                 paused;
		static mutex                             eventQueueMutex;
	};
}
#endif