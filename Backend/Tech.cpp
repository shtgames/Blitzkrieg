#include "Tag.h"

#include "Tech.h"

#include "Unit.h"
#include "TimeSystem.h"
#include "Research.h"

#include <cmath>

namespace bEnd
{
	std::unordered_map<std::string, Tech> Tech::technologies;

	const unsigned char Tech::BASE_RESEARCH_DAYS = 125;
	const float Tech::TECH_DIFFICULTY_WEIGHT = 0.1f, Tech::AHEAD_OF_TIME_PENALTY_WEIGHT = 1.5f, Tech::XP_TIME_REDUCTION_WEIGHT = 0.05f, Tech::XP_TIME_INCREASE_WEIGHT = 0.1f, Tech::XP_BREAKPOINT = 5.0f;
	
	const bool Tech::loadFromFile(const FileProcessor::Statement& source)
	{
		technologies.emplace(std::make_pair(source.lValue, Tech(source.lValue)));
		Tech& target(technologies.at(source.lValue));

		for (auto it = source.rStatements.begin(), end = source.rStatements.end(); it != end; ++it)
			if (it->lValue == "research_bonus_from")
				for (auto it1 = it->rStatements.begin(), end1 = it->rStatements.end(); it1 != end1; ++it)
					target.experienceModifierWeights[it1->lValue] = std::stof(it1->rStrings.front());
			else if (it->lValue == "on_completion")
				target.experienceReward.reset(new const std::string(it->rStrings.front()));
			else if (it->lValue == "difficulty")
				target.difficulty = std::stoi(it->rStrings.front());
			else if (it->lValue == "start_year")
				target.yearsByLevel.push_back(std::stoi(it->rStrings.front()));
			else if (it->lValue == "first_offset")
				target.yearsByLevel.push_back(std::stoi(it->rStrings.front()));
			else if (it->lValue == "additional_offset")
				for (unsigned short year = target.yearsByLevel.back(), offset = std::stoi(it->rStrings.front());
					target.yearsByLevel.size() != 12; target.yearsByLevel.push_back(year += offset));
			else if (Unit::exists(it->lValue)) {} // effects on unit...

		return true;
	}

	const unsigned short Tech::getResearchDays(const Tag& tag) const
	{
		if (!Research::exists(tag)) return -1;

		const Research& research(Research::get(tag));

		const float techLevel = research.getTechLevel(name) < getLevels() && research.getTechLevel(name) >= 0 ?
			research.getTechLevel(name) : getLevels();

		unsigned short returnValue = BASE_RESEARCH_DAYS *
			(1.0f + (difficulty / 10.0f) * TECH_DIFFICULTY_WEIGHT) *
			(1.0f + (yearsByLevel[techLevel - 1] - TimeSystem::getCurrentDate().getYear() > 0) ?
				yearsByLevel[techLevel - 1] - TimeSystem::getCurrentDate().getYear() * AHEAD_OF_TIME_PENALTY_WEIGHT : 0.0f);

		float xpModifier = 0.0f;
		for (auto it = experienceModifierWeights.begin(), end = experienceModifierWeights.end(); it != end; ++it)
		{
			const float xp = research.getExperience(it->first);
			xpModifier += (1 + (xp > XP_BREAKPOINT ? -XP_TIME_REDUCTION_WEIGHT * sqrt(xp - XP_BREAKPOINT) : XP_TIME_INCREASE_WEIGHT * (XP_BREAKPOINT - xp))) * it->second;
		}

		return returnValue * (xpModifier != 0 ? xpModifier : 1.0f);
	}	
}
