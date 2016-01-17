#ifndef TIME_SYSTEM_BACKEND
#define TIME_SYSTEM_BACKEND

#include "Event.h"

#include <memory>
#include <chrono>
#include <queue>

namespace bEnd
{
	class TimeSystem final
	{
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

		static const std::chrono::high_resolution_clock gameTime;
	private:
		static void eventCheck();

		static std::priority_queue<std::unique_ptr<EventBase>>             events;
		static unsigned char                                               gameSpeed;
		static std::chrono::time_point<std::chrono::high_resolution_clock> timeOfLastUpdate;
		static float                                                       updateIntervals[6];
		static Date                                                        currentDate;
		static bool                                                        paused;
	};
}
#endif