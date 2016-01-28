#include "Research.h"

#include "Tech.h"
#include "TimeSystem.h"

namespace bEnd
{
	unordered_map<Tag, unique_ptr<Research>> Research::research;
	const float Research::EXPERIENCE_CAP = 25.0f;

	Research::Research(const Research& copy)
		: experience(copy.experience), techLevels(copy.techLevels)
	{
		for (auto it = copy.researchQueue.begin(), end = copy.researchQueue.end(); it != end; ++it)
			researchQueue.emplace_back(new ResearchItem(**it));
	}

	const Date Research::ResearchItem::getComlpetionDate()const
	{
		if (dedicatedLeadership > 0.0f)
			return (TimeSystem::getCurrentDate() + Date((((100.0f - completionPercentage) / 100.0f) * (1.0f / dedicatedLeadership) * researchDays) * 24));
		else return Date::NEVER;
	}

	const bool Research::ResearchItem::research()
	{
		if (dedicatedLeadership > 0.0f) completionPercentage += (100.0f / (researchDays * (1.0f / dedicatedLeadership)));
		return completionPercentage >= 100.0f;
	}

	void Research::increaseResearchItemPriority(const unsigned short index)
	{
		if (index >= 1 && index < researchQueue.size())
		{
			researchQueue.at(index - 1).swap(researchQueue.at(index));
			setLeadership(totalDedicatedLeadership);
		}
	}

	void Research::decreaseResearchItemPriority(const unsigned short index)
	{
		if (index >= 0 && index < researchQueue.size() - 1)
		{
			researchQueue.at(index + 1).swap(researchQueue.at(index));
			setLeadership(totalDedicatedLeadership);
		}
	}

	void Research::setResearchItemAtMaxPriority(const unsigned short index)
	{
		if (index >= 1 && index < researchQueue.size())
		{
			researchQueue.at(0).swap(researchQueue.at(index));
			setLeadership(totalDedicatedLeadership);
		}
	}

	void Research::setResearchItemAtMinPriority(const unsigned short index)
	{
		if (index >= 0 && index < researchQueue.size() - 1)
		{
			researchQueue.at(researchQueue.size() - 1).swap(researchQueue.at(index));
			setLeadership(totalDedicatedLeadership);
		}
	}

	void Research::removeResearchItem(const unsigned short index)
	{
		if (index >= 0 && index < researchQueue.size())
		{
			if (*(researchQueue.begin() + index)) techLevels[(*(researchQueue.begin() + index))->getTech()] += (*(researchQueue.begin() + index))->getCompletionPercentage() / 100.0f;
			researchQueue.erase(researchQueue.begin() + index);
			setLeadership(totalDedicatedLeadership);
		}
	}

	void Research::addResearchItem(const std::string& element)
	{
		if (Tech::exists(element))
		{
			researchQueue.emplace_back(new ResearchItem(element, Tech::getTechnology(element).getResearchDays(techLevels[element], experience)));
			setLeadership(totalDedicatedLeadership);
		}
	}

	void Research::addExperienceRewards(const vector<pair<std::string, float>>& rewards)
	{
		for (auto it = rewards.begin(), end = rewards.end(); it != end; ++it)
			experience[it->first] + it->second > EXPERIENCE_CAP ? experience[it->first] = EXPERIENCE_CAP : experience[it->first] += it->second;
	}

	const float Research::setLeadership(float leadership)
	{
		if (leadership < 0.0f) leadership = 0.0f;

		totalDedicatedLeadership = leadership;
		for (auto it = researchQueue.begin(), end = researchQueue.end(); it != end; ++it)
			if (*it && Tech::exists((*it)->getTech()))
			{
				(*it)->setLeadership(leadership);
				(*it)->setResearchDays(Tech::getTechnology((*it)->getTech()).getResearchDays(techLevels[(*it)->getTech()], experience));
				if (leadership > 1.0f) leadership -= 1.0f;
				else leadership = 0.0f;
			}
			else it = researchQueue.erase(it);
		return leadership;
	}

	void Research::update()
	{
		for (auto it = researchQueue.begin(), end = researchQueue.end(); it != end; ++it)
			if (*it && (*it)->research())
			{
				if (Tech::exists((*it)->getTech()))
				{
					techLevels[(*it)->getTech()]++;
					addExperienceRewards(Tech::getTechnology((*it)->getTech()).getExperienceRewards());
				}
				it = researchQueue.erase(it);
			}
	}

	const bool Research::loadFromFile(ifstream& file)
	{
		return false;
	}
}