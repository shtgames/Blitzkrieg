#include "TimeSystem.h"
#include "Nation.h"

namespace bEnd
{
	std::priority_queue<std::unique_ptr<EventBase>>             TimeSystem::events;
	const std::chrono::high_resolution_clock                    TimeSystem::gameTime;
	unsigned char                                               TimeSystem::gameSpeed = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> TimeSystem::timeOfLastUpdate = TimeSystem::gameTime.now();
	float                                                       TimeSystem::updateIntervals[6];
	Date                                                        TimeSystem::currentDate = Date(0, 1, January, 1936);
	bool                                                        TimeSystem::paused = true;


	const Date& TimeSystem::getCurrentDate()
	{
		return currentDate;
	}

	void TimeSystem::update()
	{
		if (gameSpeed != 0)
		{
			if (std::chrono::duration<float>(gameTime.now() - timeOfLastUpdate).count() > updateIntervals[gameSpeed])
			{
				if (currentDate.day != ++currentDate.day) Nation::updateGlobal();
				eventCheck();
				timeOfLastUpdate = gameTime.now();
			}
		}
	}

	void TimeSystem::pause()
	{
		paused = true;
	}

	void TimeSystem::increaseSpeed()
	{
		if (gameSpeed < 5) gameSpeed++;
	}

	void TimeSystem::decreaseSpeed()
	{
		if (gameSpeed > 0) gameSpeed--;
	}

	template <typename eventFunctionType>
	void TimeSystem::addEvent(const Event<eventFunctionType>& _event)
	{
		events.push(_event);
	}

	template <typename eventFunctionType>
	void TimeSystem::addEvent(Event<eventFunctionType>&& _event)
	{
		events.emplace(_event);
	}

	void TimeSystem::reset(const Date& _Date)
	{
		currentDate = _Date;
	}

	void TimeSystem::eventCheck()
	{
		if (events.top()->trigger == currentDate)
		{
			events.top()->initiate();
			events.pop();
		}
	}
}