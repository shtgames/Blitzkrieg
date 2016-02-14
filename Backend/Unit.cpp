#include "Tag.h"

#include "Unit.h"

namespace bEnd
{
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

	const bool Unit::loadFromFile(const std::string & file)
	{
		return false;
	}

	Unit::Unit(const std::string& name)
		: name(name)
	{
		for (unsigned char it = 0; it < Resource::Last; it++)
			ResourceAdditionAndMultiplier[(Resource)it];
	}
}