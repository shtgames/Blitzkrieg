#ifndef UNIT_BACKEND
#define UNIT_BACKEND

#include "Resources.h"
#include "FileProcessor.h"

#include <unordered_map>
#include <set>
#include <memory>
#include <map>
#include <string>

namespace bEnd
{
	class Tag;
	class Unit final
	{
	public:
		enum Type
		{
			Building,
			Land,
			Air,
			Naval
		};

		Unit(const Unit&) = default;
		Unit(Unit&&) = default;
		~Unit() = default;

		const float getRequiredIC(const Tag& tag)const;
		const float getRequiredManpower()const;
		const unsigned short getProductionDays(const Tag& tag)const;
		const Type getType()const { return type; }
		const std::unique_ptr<const std::string>& getExperienceReward()const { return experienceReward; }
		const bool isDelayDeployable()const { return canAwaitDeployment; }

		const std::string& getName()const { return name; }
		const float getICAddition()const { return ICAddition; }
		const float getICMultiplier()const { return ICMultiplier; }
		const float getLeadershipAddition()const { return LeadershipAddition; }
		const float getLeadershipMultiplier()const { return LeadershipMultiplier; }
		const float getResourceAddition(const Resource resource)const { return ResourceAdditionAndMultiplier[resource].first; }
		const float getResourceMultiplier(const Resource resource)const { return ResourceAdditionAndMultiplier[resource].second; }
		const float getManpowerAddition()const { return ManpowerAddition; }
		const float getManpowerMultiplier()const { return ManpowerMultiplier; }
		const bool isCoastal()const { return coastal; }

		static const bool loadFromFile(const FileProcessor::Statement& file);
		static const bool exists(const std::string& unit) { if (units.count(unit)) return true; return false; }
		static const Unit& get(const std::string& unit) { return units.at(unit); }
		static std::set<std::string>::const_iterator begin();
		static std::set<std::string>::const_iterator end();
		static const unsigned short unitsOfType(const Type type);

		static void load();

	private:
		Unit() = delete;
		Unit(const std::string& name);

		// General
		const std::string name;
		float baseRequiredIC = 0.0f;
		float baseBuildTime = 0.0f;
		float baseRequiredManpower = 0.0f;
		Type type = Land;
		bool canAwaitDeployment = false;
		std::map<std::string, float> experienceModifierWeights;
		std::unique_ptr<const std::string> experienceReward;

		// Building
		float ICAddition = 0.0f, ICMultiplier = 1.0f, LeadershipAddition = 0.0f, LeadershipMultiplier = 1.0f,
			ManpowerAddition = 0.0f, ManpowerMultiplier = 1.0f;
		mutable std::unordered_map<Resource, std::pair<float, float>> ResourceAdditionAndMultiplier;
		unsigned short lineOfSightAddition = 0;
		bool coastal = false;

		// Land
		bool canParadrop = false;
		
		static std::set<std::string> iterable;
		static std::unordered_map<std::string, Unit> units;
		static std::unordered_map<unsigned char, unsigned short> unitCount;
	};
}

#endif
