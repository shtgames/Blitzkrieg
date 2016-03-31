#include "Research.h"

#include "Tag.h"
#include "Tech.h"
#include "TimeSystem.h"

namespace bEnd
{
	unordered_map<Tag, unique_ptr<Research>> Research::research;
	const float Research::EXPERIENCE_CAP = 25.0f, Research::EXPERIENCE_DECAY_PERCENTAGE = 0.025f;

	const Date Research::ResearchItem::getComlpetionDate()const
	{
		if (dedicatedLeadership > 0.0f)
			return (TimeSystem::getCurrentDate() + (((100.0f - completionPercentage) / 100.0f) * (1.0f / dedicatedLeadership) * researchDays) * 24);
		else return Date::NEVER;
	}

	Research::ResearchItem& Research::ResearchItem::updateResearchDays(const Tag& tag)
	{
		researchDays = tech.getResearchDays(tag);
		return *this;
	}

	const bool Research::ResearchItem::research()
	{
		if (dedicatedLeadership > 0.0f) completionPercentage = completionPercentage + (100.0f / (researchDays * (1.0f / dedicatedLeadership)));
		return completionPercentage >= 100.0f;
	}

	void Research::increaseResearchItemPriority(const unsigned short index)
	{
		researchQueueLock.lock();
		if (index != 0 && index < researchQueue.size())
		{
			researchQueue.at(index - 1).swap(researchQueue.at(index));
			setLeadership(totalDedicatedLeadership);
		}
		researchQueueLock.unlock();
	}

	void Research::decreaseResearchItemPriority(const unsigned short index)
	{
		researchQueueLock.lock();
		if (index < researchQueue.size() - 1)
		{
			researchQueue.at(index + 1).swap(researchQueue.at(index));
			setLeadership(totalDedicatedLeadership);
		}
		researchQueueLock.unlock();
	}

	void Research::setResearchItemAtMaxPriority(const unsigned short index)
	{
		researchQueueLock.lock();
		if (index != 0 && index < researchQueue.size())
		{
			researchQueue.at(0).swap(researchQueue.at(index));
			setLeadership(totalDedicatedLeadership);
		}
		researchQueueLock.unlock();
	}

	void Research::setResearchItemAtMinPriority(const unsigned short index)
	{
		researchQueueLock.lock();
		if (index < researchQueue.size() - 1)
		{
			researchQueue.back().swap(researchQueue.at(index));
			setLeadership(totalDedicatedLeadership);
		}
		researchQueueLock.unlock();
	}

	void Research::removeResearchItem(const unsigned short index)
	{
		researchQueueLock.lock();
		if (index < researchQueue.size())
		{
			if (*(researchQueue.begin() + index)) 
				techLevels[(*(researchQueue.begin() + index))->getTech().getName()] = (*(researchQueue.begin() + index))->getCompletionPercentage() / 100.0f
					+ techLevels[(*(researchQueue.begin() + index))->getTech().getName()];
			researchQueue.erase(researchQueue.begin() + index);
			setLeadership(totalDedicatedLeadership);
		}
		researchQueueLock.unlock();
	}

	void Research::addResearchItem(const std::string& element)
	{
		if (Tech::exists(element) && techLevels[element] < Tech::get(element).getLevels())
		{
			researchQueueLock.lock();
			researchQueue.emplace_back(new ResearchItem(Tech::get(element),
				Tech::get(element).getResearchDays(tag),
				(techLevels[element] - int(techLevels[element])) * 100.0f));
			researchQueueLock.unlock();
			setLeadership(totalDedicatedLeadership);
		}
	}

	void Research::addExperienceRewards(const std::string& target, const float amount)
	{
		experienceLock.lock();
		experience[target] + amount > EXPERIENCE_CAP ? experience[target] = EXPERIENCE_CAP : experience[target] = amount + experience[target];
		experienceHasBeenAdded[target] = true;
		experienceLock.unlock();
	}

	const float Research::setLeadership(float leadership)
	{
		if (leadership < 0.0f) leadership = 0.0f;

		totalDedicatedLeadership = leadership;
		researchQueueLock.lock();
		for (auto it = researchQueue.begin(), end = researchQueue.end(); it != end; ++it)
			if (*it)
			{
				(*it)->setLeadership(leadership);
				leadership > 1.0f ? leadership -= 1.0f : leadership = 0.0f;
			}
			else it = researchQueue.erase(it);
		researchQueueLock.unlock();
		return leadership;
	}

	void Research::update()
	{
		researchQueueLock.lock();
		for (auto it = researchQueue.begin(), end = researchQueue.end(); it != end; ++it)
			if (*it)
			{
				(*it)->updateResearchDays(tag);
				if ((*it)->research())
				{
					techLevels[(*it)->getTech().getName()] + 1 > (*it)->getTech().getLevels() ? 
						techLevels[(*it)->getTech().getName()] = (*it)->getTech().getLevels() :
						techLevels[(*it)->getTech().getName()] = techLevels[(*it)->getTech().getName()] + 1;
					if ((*it)->getTech().getExperienceReward())
						addExperienceRewards(*(*it)->getTech().getExperienceReward(), 1.0f);
					it = researchQueue.erase(it);
				}
			}
			else it = researchQueue.erase(it);
		researchQueueLock.unlock();

		experienceLock.lock();
		for (auto it = experience.begin(), end = experience.end(); it != end; ++it)
			if (experienceHasBeenAdded[it->first]) experienceHasBeenAdded[it->first] = false;
			else it->second = it->second * ( 1.0f - EXPERIENCE_DECAY_PERCENTAGE);
		experienceLock.unlock();
	}

	void Research::loadTechLevels(const FileProcessor::Statement& source)
	{
		for (auto it = source.rStatements.begin(), end = source.rStatements.end(); it != end; ++it)
			if (it->lValue == "technology")
			{
				for (auto it = source.rStatements.begin(), end = source.rStatements.end(); it != end; ++it)
					if (it->lValue == "notech") continue;
					else if (Tech::exists(it->lValue))
						techLevels[it->lValue] = std::stoi(it->rStrings.at(0)) + std::stof(it->rStrings.at(1));
			}
	}

	void Research::loadResearchItem(const FileProcessor::Statement& source)
	{
		if (source.lValue == "research")
			addResearchItem(source.rStrings.front());
	}

	void Research::loadExperienceLevels(const FileProcessor::Statement& source)
	{
		if (source.lValue == "nocategory") return;
		else experience[source.lValue] = std::stof(source.rStrings.front());
	}
}