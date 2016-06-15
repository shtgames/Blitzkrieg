#include "TimeSystem.h"

#include "Nation.h"
#include "ResourceDistributor.h"

namespace bEnd
{
	std::mutex                          TimeSystem::eventQueueMutex;
	priority_queue<unique_ptr<Event>>   TimeSystem::events;
	const chrono::high_resolution_clock TimeSystem::gameTime;
	std::atomic<unsigned char>          TimeSystem::gameSpeed = 4;
	TimePoint                           TimeSystem::timeOfLastUpdate = TimeSystem::gameTime.now();
	const std::vector<float>            TimeSystem::updateIntervals = { 4.0f, 2.0f, 1.0f, 0.5f, 0.042f };
	Date                                TimeSystem::currentDate = Date(0, 1, January, 1936);
	std::atomic<bool>                   TimeSystem::paused = true;
	
	const bool TimeSystem::isPaused()
	{
		return paused;
	}

	const unsigned char TimeSystem::getSpeed()
	{
		return gameSpeed;
	}

	const Date& TimeSystem::getCurrentDate()
	{
		return currentDate;
	}

	void TimeSystem::update()
	{
		if (!paused && Duration(gameTime.now() - timeOfLastUpdate).count() > updateIntervals.at(gameSpeed))
		{
			if (currentDate.getDay() != (++currentDate).getDay())
				for (auto& it : ResourceDistributor::resourceDistributors)
					it.second->update();
			eventCheck();
			timeOfLastUpdate = gameTime.now();
		}
	}

	void TimeSystem::pause()
	{
		paused = true;
	}

	void TimeSystem::resume()
	{
		paused = false;
	}

	void TimeSystem::increaseSpeed()
	{
		if (gameSpeed < updateIntervals.size() - 1) gameSpeed++;
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
		paused = true;
		gameSpeed = 0;
		eventQueueMutex.lock();
		while (!events.empty()) events.pop();
		eventQueueMutex.unlock();
	}

	void TimeSystem::eventCheck()
	{
		eventQueueMutex.lock();
		if (!events.empty() && events.top()->trigger == currentDate)
		{
			events.top()->initiate();
			events.pop();
		}
		eventQueueMutex.unlock();
	}
}