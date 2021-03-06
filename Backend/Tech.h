#ifndef TECH_BACKEND
#define TECH_BACKEND

#include "Date.h"
#include "FileProcessor.h"

#include <unordered_map>
#include <map>
#include <vector>
#include <fstream>
#include <string>
#include <memory>
#include <atomic>
#include <mutex>

namespace bEnd
{
	class Tag;
	class Tech final
	{
	public:

		Tech(const Tech&) = default;
		Tech(Tech&&) = default;
		~Tech() = default;

		const unsigned char getDifficulty()const { return difficulty; }
		const unsigned char getLevels()const { return yearsByLevel.size(); }
		const unsigned short getYear(const unsigned char level)const { return level < yearsByLevel.size() ? yearsByLevel[level] : 0; }
		const std::unique_ptr<const std::string>& getExperienceReward()const { return experienceReward; }
		const std::string& getName()const { return name; }

		const unsigned short getResearchDays(const Tag& tag)const;

		static const bool loadFromFile(const FileProcessor::Statement& source);
		static const bool exists(const std::string& tech) { if (technologies.count(tech)) return true; return false; }
		static const Tech& get(const std::string& tech) { return technologies.at(tech); }

	private:

		Tech() = delete;
		Tech(const std::string& name) : name(name) {}

		const std::string name;
		unsigned char difficulty = 0;
		std::vector<unsigned short> yearsByLevel;
		std::map<std::string, float> experienceModifierWeights;
		std::unique_ptr<const std::string> experienceReward = nullptr;

		static const unsigned char BASE_RESEARCH_DAYS;
		static const float TECH_DIFFICULTY_WEIGHT, AHEAD_OF_TIME_PENALTY_WEIGHT, XP_TIME_REDUCTION_WEIGHT, XP_TIME_INCREASE_WEIGHT, XP_BREAKPOINT;

		static std::unordered_map<std::string, Tech> technologies;
	};
}

#endif