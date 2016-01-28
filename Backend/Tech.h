#ifndef TECH_BACKEND
#define TECH_BACKEND

#include "Date.h"

#include <unordered_map>
#include <map>
#include <vector>
#include <fstream>
#include <string>

namespace bEnd
{
	class Tech final
	{
	public:

		~Tech() = default;

		const unsigned char getDifficulty()const { return difficulty; }
		const unsigned char getLevels()const { return yearsByLevel.size(); }
		const unsigned short getYear(const unsigned char level)const { return level < yearsByLevel.size() ? yearsByLevel[level] : 0; }
		const std::vector<std::pair<std::string, float>>& getExperienceRewards()const { return experienceRewards; }

		const unsigned short getResearchDays(const unsigned char level, const std::map<std::string, float>& practicalAndTheoreticalKnowledge)const;

		static const bool loadFromFile(std::ifstream&);
		static const bool exists(const std::string& tech) { if (technologies.count(tech) && technologies[tech]) return true; return false; }
		static const Tech& getTechnology(const std::string& tech) { return *technologies[tech]; }

	private:

		Tech(const Tech&) = default;
		Tech(Tech&&) = default;
		Tech() = default;

		unsigned char difficulty = 0;
		std::vector<unsigned short> yearsByLevel;
		std::map<std::string, float> experienceModifierWeights;
		std::vector<std::pair<std::string, float>> experienceRewards;

		static const unsigned char BASE_RESEARCH_DAYS;
		static const float TECH_DIFFICULTY_WEIGHT, AHEAD_OF_TIME_PENALTY_WEIGHT, XP_TIME_REDUCTION_WEIGHT, XP_TIME_INCREASE_WEIGHT, XP_BREAKPOINT;

		static std::unordered_map<std::string, unique_ptr<Tech>> technologies;
	};
}

#endif