#include "TimeSystem.h"

#include "Nation.h"

namespace bEnd
{
	priority_queue<unique_ptr<TimeSystem::EventBase>> TimeSystem::events;
	const chrono::high_resolution_clock               TimeSystem::gameTime;
	std::atomic<unsigned char>                        TimeSystem::gameSpeed = 0;
	chrono::time_point<chrono::high_resolution_clock> TimeSystem::timeOfLastUpdate = TimeSystem::gameTime.now();
	const std::vector<float>                          TimeSystem::updateIntervals = { -1.0f, 5.0f, 2.0f, 1.0f, 0.5f, 0.042f };
	Date                                              TimeSystem::currentDate = Date(0, 1, January, 1936);
	std::atomic<bool>                                 TimeSystem::paused = true;



	const Date& TimeSystem::getCurrentDate()
	{
		dateMutex.lock();
		return currentDate;
		dateMutex.unlock();
	}

	void TimeSystem::update()
	{
		if (gameSpeed != 0 && (gameTime.now() - timeOfLastUpdate).count() > updateIntervals.at(gameSpeed))
		{
			dateMutex.lock();
			const bool condition = currentDate.getDay() != (++currentDate).getDay();
			dateMutex.unlock();
			if (condition) Nation::updateGlobal();
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

	template <typename eventFunctionType>
	void TimeSystem::addEvent(const Event<eventFunctionType>& _event)
	{
		eventQueueMutex.lock();
		events.push(_event);
		eventQueueMutex.unlock();
	}

	template <typename eventFunctionType>
	void TimeSystem::addEvent(Event<eventFunctionType>&& _event)
	{
		eventQueueMutex.lock();
		events.emplace(std::move(_event));
		eventQueueMutex.lock();
	}

	void TimeSystem::reset(const Date& date)
	{
		dateMutex.lock();
		currentDate = date;
		dateMutex.unlock();
	}

	void TimeSystem::eventCheck()
	{
		eventQueueMutex.lock();
		dateMutex.lock();
		const bool condition = events.top()->trigger == currentDate;
		dateMutex.unlock();
		if (condition)
		{
			events.top()->initiate();
			events.pop();
		}
		eventQueueMutex.unlock();
	}
}