#include "Tag.h"

#include "Unit.h"

namespace bEnd
{
	std::unordered_map<std::string, Unit> Unit::units;

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
			}
			else if (it->lValue == "can_paradrop" && it->rStrings.front() == "yes")
				target.canParadrop = true;
			else if (it->lValue == "build_cost_ic")
				target.baseRequiredIC = std::stof(it->rStrings.front());
			else if (it->lValue == "build_cost_manpower")
				target.baseRequiredManpower = std::stof(it->rStrings.front());
			else if (it->lValue == "build_time")
				target.baseBuildTime = std::stoi(it->rStrings.front());
			///
		return true;
	}

	Unit::Unit(const std::string& name)
		: name(name)
	{
		for (unsigned char it = 0; it < Resource::Last; it++)
			ResourceAdditionAndMultiplier[(Resource)it];
	}
}