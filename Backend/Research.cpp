#include "Research.h"

#include "Tech.h"
#include "TimeSystem.h"

namespace bEnd
{
	const Date Research::ResearchItem::getComlpetionDate()const
	{
		if (dedicatedLeadership > 0.0f)
			return (TimeSystem::getCurrentDate() + Date((((100.0f - completionPercentage) / 100.0f) * (1.0f / dedicatedLeadership) * researchDays) * 24));
		else return Date();
	}

	const bool Research::ResearchItem::research()
	{
		if (dedicatedLeadership > 0.0f) completionPercentage += (100.0f / (researchDays * (1.0f / dedicatedLeadership)));
		return completionPercentage >= 100.0f;
	}

	void Research::increaseResearchItemPriority(const unsigned short index)
	{
		if (index >= 1 && index < researchQueue.size())
			researchQueue.at(index - 1).swap(researchQueue.at(index));
	}

	void Research::decreaseResearchItemPriority(const unsigned short index)
	{
		if (index >= 0 && index < researchQueue.size() - 1)
			researchQueue.at(index + 1).swap(researchQueue.at(index));
	}

	void Research::setResearchItemAtMaxPriority(const unsigned short index)
	{
		if (index >= 1 && index < researchQueue.size())
			researchQueue.at(0).swap(researchQueue.at(index));
	}

	void Research::setResearchItemAtMinPriority(const unsigned short index)
	{
		if (index >= 0 && index < researchQueue.size() - 1)
			researchQueue.at(researchQueue.size() - 1).swap(researchQueue.at(index));
	}

	void Research::removeResearchItem(const unsigned short index)
	{
		if (index >= 0 && index < researchQueue.size())
		{
			if (*(researchQueue.begin() + index)) techLevels[(*(researchQueue.begin() + index))->getTech()] += (*(researchQueue.begin() + index))->getCompletionPercentage() / 100.0f;
			researchQueue.erase(researchQueue.begin() + index);
		}
	}

	void Research::addResearchItem(const std::string& element)
	{
		if (Tech::getTechnologies().count(element))
			researchQueue.push_back(std::unique_ptr<ResearchItem>(&new ResearchItem(element)->setResearchDays(Tech::getTechnologies().at(element).getResearchDays(techLevels[element], experience))));
	}

	const float Research::setLeadership(float leadership)
	{
		for (auto it = researchQueue.begin(), end = researchQueue.end(); it != end; ++it)
			if (*it)
			{
				(*it)->setLeadership(leadership);
				(*it)->setResearchDays(Tech::getTechnologies().at((*it)->getTech()).getResearchDays(techLevels[(*it)->getTech()], experience));
				leadership > 1.0f ? leadership -= 1.0f : leadership = 0.0f;
			}
		return leadership > 0 ? leadership : 0.0f;
	}

	void Research::update()
	{
		for (auto it = researchQueue.begin(), end = researchQueue.end(); it != end; ++it)
			if ((*it) && (*it)->research())
			{
				techLevels[(*it)->getTech()]++;
				for (auto it1 = Tech::getTechnologies().at((*it)->getTech()).getExperienceRewards().begin(), end1 = Tech::getTechnologies().at((*it)->getTech()).getExperienceRewards().begin(); it1 != end1; ++it)
					experience[it1->first] += it1->second;
				it = researchQueue.erase(it);
			}
	}

	const bool Research::loadFromFile(ifstream& file)
	{
		return false;
	}
}