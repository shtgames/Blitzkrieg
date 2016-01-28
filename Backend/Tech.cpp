#include "Tech.h"

#include "TimeSystem.h"

namespace bEnd
{
	std::unordered_map<std::string, unique_ptr<Tech>> Tech::technologies;

	const unsigned char Tech::BASE_RESEARCH_DAYS = 125;
	const float Tech::TECH_DIFFICULTY_WEIGHT = 0.1f, Tech::AHEAD_OF_TIME_PENALTY_WEIGHT = 1.0f, Tech::XP_TIME_REDUCTION_WEIGHT = 0.05f, Tech::XP_TIME_INCREASE_WEIGHT = 0.1f, Tech::XP_BREAKPOINT = 5.0f;
	
	const bool Tech::loadFromFile(std::ifstream& file)
	{
		return false;
	}

	const unsigned short Tech::getResearchDays(const unsigned char level, const std::map<std::string, float>& xpMods) const
	{
		unsigned short returnValue = BASE_RESEARCH_DAYS *
			(1.0f + difficulty * TECH_DIFFICULTY_WEIGHT) *
			(1.0f + yearsByLevel[level] - TimeSystem::getCurrentDate().getYear() > 0 ? yearsByLevel[level] - TimeSystem::getCurrentDate().getYear() * AHEAD_OF_TIME_PENALTY_WEIGHT : 0.0f);

		float xpModifier = 0.0f;
		for (auto it = experienceModifierWeights.begin(), end = experienceModifierWeights.end(); it != end; ++it)
			xpModifier += (1.0f + (xpMods.at(it->first) > XP_BREAKPOINT ? -XP_TIME_REDUCTION_WEIGHT * sqrt(xpMods.at(it->first) - XP_BREAKPOINT) : XP_TIME_INCREASE_WEIGHT * (XP_BREAKPOINT - xpMods.at(it->first)))) * it->second;

		return returnValue * xpModifier;
	}	
}