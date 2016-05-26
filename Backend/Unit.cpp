#include "Tag.h"

#include "Unit.h"

#include "FileProcessor.h"

namespace bEnd
{
	std::unordered_map<std::string, Unit> Unit::units;
	std::unordered_map<Unit::Type, unsigned short> Unit::unitCount;

	const float Unit::getRequiredIC(const Tag& tag)const
	{
		return baseRequiredIC; // modifiers...
	}

	const float Unit::getRequiredManpower() const
	{
		return baseRequiredManpower; // modifiers...
	}

	const unsigned short Unit::getProductionDays(const Tag& tag) const
	{
		return baseBuildTime; // modifiers...
	}

	const bool Unit::loadFromFile(const FileProcessor::Statement& source)
	{
		units.emplace(std::make_pair(source.lValue, Unit(source.lValue)));
		Unit& target(units.at(source.lValue));

		for (auto it = source.rStatements.begin(), end = source.rStatements.end(); it != end; ++it)
			if (it->lValue == "type")
			{
				if (it->rStrings.front() == "land") target.type = Land;
				else if (it->rStrings.front() == "air") target.type = Air;
				else if (it->rStrings.front() == "naval") target.type = Naval;
				else if (it->rStrings.front() == "building") target.type = Building;
				unitCount[target.type]++;
			}
			else if (it->lValue == "can_paradrop" && it->rStrings.front() == "yes")
				target.canParadrop = true;
			else if (it->lValue == "build_cost_ic")
				target.baseRequiredIC = std::stof(it->rStrings.front());
			else if (it->lValue == "build_cost_manpower")
				target.baseRequiredManpower = std::stof(it->rStrings.front());
			else if (it->lValue == "build_time")
				target.baseBuildTime = std::stoi(it->rStrings.front());
			else if (it->lValue == "ic_add")
				target.ICAddition = std::stof(it->rStrings.front());
			else if (it->lValue == "ic_mult")
				target.ICMultiplier = std::stof(it->rStrings.front());
			else if (it->lValue == "ls_add")
				target.LeadershipAddition = std::stof(it->rStrings.front());
			else if (it->lValue == "ls_mult")
				target.LeadershipMultiplier = std::stof(it->rStrings.front());
			else if (it->lValue == "mp_add")
				target.ManpowerAddition = std::stof(it->rStrings.front());
			else if (it->lValue == "mp_mult")
				target.ManpowerMultiplier = std::stof(it->rStrings.front());
			else if (it->lValue == "resource_mod")
				for (const auto& it1 : it->rStatements)
				{
					Resource resource;
					if (it1.lValue == "energy") resource = Energy;
					else if (it1.lValue == "metal") resource = Metal;
					else if (it1.lValue == "rare_mats") resource = RareMaterials;
					else if (it1.lValue == "oil") resource = CrudeOil;
					else if (it1.lValue == "supplies") resource = Supplies;
					else if (it1.lValue == "fuel") resource = Fuel;
					else if (it1.lValue == "money") resource = Money;
					target.ResourceAdditionAndMultiplier[resource].first = std::stof(it1.rStrings.at(0));
					target.ResourceAdditionAndMultiplier.at(resource).second = std::stof(it1.rStrings.at(1));
				}
		return true;
	}

	std::unordered_map<std::string, Unit>::const_iterator Unit::begin()
	{
		return units.begin();
	}

	std::unordered_map<std::string, Unit>::const_iterator Unit::end()
	{
		return units.end();
	}

	const unsigned short Unit::unitsOfType(const Type type)
	{
		return unitCount[type];
	}

	Unit::Unit(const std::string& name)
		: name(name) 
	{
		for (auto it(0); it != Resource::Last; ++it)
			ResourceAdditionAndMultiplier[(Resource)it].second = 1.0f;
	}

	void Unit::load()
	{
		for (const auto& it : getDirectoryContents("units/*.txt"))
		{
			FileProcessor source("units/" + it);
			if (!source.isOpen()) continue;
			loadFromFile(source.getStatements().front());
		}
	}
}