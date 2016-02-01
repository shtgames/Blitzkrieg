#ifndef UNIT_BACKEND
#define UNIT_BACKEND

#include "Resources.h"

#include <unordered_map>
#include <memory>
#include <map>

namespace bEnd
{
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
		
		~Unit() = default;

		const float getRequiredIC(const std::map<std::string, float>&)const;
		const float getRequiredManpower()const;
		const unsigned short getProductionDays(const std::map<std::string, float>& practicalAndTheoreticalKnowledge)const;
		const Type getType()const { return type; }
		const std::vector<std::pair<std::string, float>>& getExperienceRewards()const { return experienceRewards; }
		const bool isDelayDeployable()const { return canAwaitDeployment; }

		const std::string& getName()const { return name; }
		const float getICAddition()const { return 0.0f; }
		const float getICMultiplier()const { return 0.0f; }
		const float getLeadershipAddition()const { return 0.0f; }
		const float getLeadershipMultiplier()const { return 0.0f; }
		const float getResourceAddition(const Resource resource)const { return 0.0f; }
		const float getResourceMultiplier(const Resource resource)const { return 0.0f; }
		const float getManpowerAddition()const { return 0.0f; }
		const float getManpowerMultiplier()const { return 0.0f; }

		static const bool loadFromFile(const std::string& file);
		static const bool exists(const std::string& unit) { if (units.count(unit) && units.at(unit)) return true; return false; }
		static const Unit& get(const std::string& unit) { return *units.at(unit); }

	private:

		Unit(const Unit&) = default;
		Unit(Unit&&) = default;
		Unit() = delete;
		Unit(const std::string& name) : name(name) {}

		const std::string name;
		float baseRequiredIC;
		float baseRequiredManpower;
		float baseBuildTime;
		Type type;
		bool canAwaitDeployment;
		std::map<std::string, float> experienceModifierWeights;
		std::vector<std::pair<std::string, float>> experienceRewards;

		static std::unordered_map<std::string, std::unique_ptr<Unit>> units;
	};
}

#endif