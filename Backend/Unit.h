#ifndef UNIT_BACKEND
#define UNIT_BACKEND

#include <unordered_map>
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
		const unsigned short getProductionDays(const std::map<std::string, float>& practicalAndTheoreticalKnowledge)const;
		const Type getType()const { return type; }
		const std::vector<std::pair<std::string, float>>& getExperienceRewards()const { return experienceRewards; }
		const bool isDelayDeployable()const { return canAwaitDeployment; }

		static const bool loadFromFile(const std::string& file);
		static const bool exists(const std::string& unit) { if (units.count(unit) && units.at(unit)) return true; return false; }
		static const Unit& getUnit(const std::string& unit) { return *units.at(unit); }

	private:

		Unit(const Unit&) = default;
		Unit(Unit&&) = default;
		Unit() = default;

		float baseRequiredIC;
		Type type;
		bool canAwaitDeployment;
		std::map<std::string, float> experienceModifierWeights;
		std::vector<std::pair<std::string, float>> experienceRewards;

		static std::unordered_map<std::string, unique_ptr<Unit>> units;
	};
}

#endif