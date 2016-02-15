#include "TimeSystem.h"

namespace bEnd
{
	priority_queue<unique_ptr<Event>>                 TimeSystem::events;
	const chrono::high_resolution_clock               TimeSystem::gameTime;
	std::atomic<unsigned char>                        TimeSystem::gameSpeed = 0;
	chrono::time_point<chrono::high_resolution_clock> TimeSystem::timeOfLastUpdate = TimeSystem::gameTime.now();
	const std::vector<float>                          TimeSystem::updateIntervals = { -1.0f, 5.0f, 2.0f, 1.0f, 0.5f, 0.042f };
	Date                                              TimeSystem::currentDate = Date(0, 1, January, 1936);
	std::atomic<bool>                                 TimeSystem::paused = true;
	
	const Date& TimeSystem::getCurrentDate()
	{
		return currentDate;
	}

	void TimeSystem::update()
	{
		if (gameSpeed != 0 && (gameTime.now() - timeOfLastUpdate).count() > updateIntervals.at(gameSpeed))
		{
			if (currentDate.getDay() != (++currentDate).getDay()) 0;
			eventCheck();
			timeOfLastUpdate = gameTime.now();
		}
	}

	void TimeSystem::pause()
	{
		paused = true;
	}

	void TimeSystem::increaseSpeed()
	{
		if (gameSpeed < updateIntervals.size()) gameSpeed++;
	}

	void TimeSystem::decreaseSpeed()
	{
		if (gameSpeed > 0) gameSpeed--;
	}

	void TimeSystem::addEvent(const Event& event)
	{
		eventQueueMutex.lock();
		events.emplace(new Event(event));
		eventQueueMutex.unlock();
	}

	void TimeSystem::addEvent(Event&& _event)
	{
		eventQueueMutex.lock();
		events.emplace(new Event(std::move(_event)));
		eventQueueMutex.lock();
	}

	void TimeSystem::reset(const Date& date)
	{
		currentDate = date;
	}

	void TimeSystem::eventCheck()
	{
		eventQueueMutex.lock();
		if (events.top()->trigger == currentDate)
		{
			events.top()->initiate();
			events.pop();
		}
		eventQueueMutex.unlock();
	}
}