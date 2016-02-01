#include "Research.h"

#include "Tech.h"
#include "TimeSystem.h"

namespace bEnd
{
	unordered_map<Tag, unique_ptr<Research>> Research::research;
	const float Research::EXPERIENCE_CAP = 25.0f, Research::EXPERIENCE_DECAY_PERCENTAGE = 0.025f;

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

	Research::ResearchItem& Research::ResearchItem::updateResearchDays(const Tag& tag)
	{
		researchDays = tech.getResearchDays(Research::get(tag).getTechLevel(tech.getName()), Research::get(tag).getExperience());
		return *this;
	}

	const bool Research::ResearchItem::research()
	{
		if (dedicatedLeadership > 0.0f) completionPercentage += (100.0f / (researchDays * (1.0f / dedicatedLeadership)));
		return completionPercentage >= 100.0f;
	}

	void Research::increaseResearchItemPriority(const unsigned short index)
	{
		if (index != 0 && index < researchQueue.size())
		{
			researchQueue.at(index - 1).swap(researchQueue.at(index));
			setLeadership(totalDedicatedLeadership);
		}
	}

	void Research::decreaseResearchItemPriority(const unsigned short index)
	{
		if (index < researchQueue.size() - 1)
		{
			researchQueue.at(index + 1).swap(researchQueue.at(index));
			setLeadership(totalDedicatedLeadership);
		}
	}

	void Research::setResearchItemAtMaxPriority(const unsigned short index)
	{
		if (index != 0 && index < researchQueue.size())
		{
			researchQueue.at(0).swap(researchQueue.at(index));
			setLeadership(totalDedicatedLeadership);
		}
	}

	void Research::setResearchItemAtMinPriority(const unsigned short index)
	{
		if (index < researchQueue.size() - 1)
		{
			researchQueue.back().swap(researchQueue.at(index));
			setLeadership(totalDedicatedLeadership);
		}
	}

	void Research::removeResearchItem(const unsigned short index)
	{
		if (index < researchQueue.size())
		{
			if (*(researchQueue.begin() + index)) 
				techLevels[(*(researchQueue.begin() + index))->getTech().getName()] += (*(researchQueue.begin() + index))->getCompletionPercentage() / 100.0f;
			researchQueue.erase(researchQueue.begin() + index);
			setLeadership(totalDedicatedLeadership);
		}
	}

	void Research::addResearchItem(const std::string& element)
	{
		if (Tech::exists(element) && techLevels[element] < Tech::get(element).getLevels())
		{
			researchQueue.emplace_back(new ResearchItem(Tech::get(element),
				Tech::get(element).getResearchDays(techLevels.at(element), experience),
				(techLevels[element] - int(techLevels[element])) * 100.0f));
			setLeadership(totalDedicatedLeadership);
		}
	}

	void Research::addExperienceRewards(const vector<pair<std::string, float>>& rewards)
	{
		for (auto it = rewards.begin(), end = rewards.end(); it != end; ++it)
		{
			experience[it->first] + it->second > EXPERIENCE_CAP ? experience[it->first] = EXPERIENCE_CAP : experience[it->first] += it->second;
			experienceHasBeenAdded[it->first] = true;
		}
	}

	const float Research::setLeadership(float leadership)
	{
		if (leadership < 0.0f) leadership = 0.0f;

		totalDedicatedLeadership = leadership;
		for (auto it = researchQueue.begin(), end = researchQueue.end(); it != end; ++it)
			if (*it)
			{
				(*it)->setLeadership(leadership);
				leadership > 1.0f ? leadership -= 1.0f : leadership = 0.0f;
			}
			else it = researchQueue.erase(it);
		return leadership;
	}

	void Research::update()
	{
		for (auto it = researchQueue.begin(), end = researchQueue.end(); it != end; ++it)
			if (*it)
			{
				(*it)->updateResearchDays(tag);
				if ((*it)->research())
				{
					techLevels[(*it)->getTech().getName()] + 1 > (*it)->getTech().getLevels() ? techLevels[(*it)->getTech().getName()] = (*it)->getTech().getLevels() : techLevels[(*it)->getTech().getName()]++;
					addExperienceRewards((*it)->getTech().getExperienceRewards());
					it = researchQueue.erase(it);
				}
			}
			else it = researchQueue.erase(it);

		for (auto it = experience.begin(), end = experience.end(); it != end; ++it)
			if (experienceHasBeenAdded[it->first]) experienceHasBeenAdded[it->first] = false;
			else it->second -= it->second * EXPERIENCE_DECAY_PERCENTAGE;
	}

	const bool Research::loadFromFile(ifstream& file)
	{
		return false;
	}
}