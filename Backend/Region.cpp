#include "Region.h"

#include "Politics.h"
#include "Production.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <math.h>
#include <thread>

const std::string CACHEDIR = "map/cache/provinces.bin";

using namespace std;

namespace bEnd
{
	const float Region::ANNEXED_NON_CORE_PENALTY = 0.7f, Region::IC_POINTS_PER_LEVEL = 1.0f;

	void Region::changeOwner(const Tag& tag)
	{
		owner = tag;
		addCore(owner);
	}

	void Region::changeController(const Tag& tag)
	{
		controller = tag;
	}

	void Region::addCore(const Tag& tag)
	{
		cores.emplace(tag);
	}

	void Region::build(const std::string& building)
	{
		BuildingLevels& buffer = buildings[building];
		if (buffer.second <= 9.0f)
		{
			buffer.first++;
			buffer.second++;
		}
	}

	const bool Region::loadFromFile(ifstream& file)
	{
		return false;
	}
	
	const float Region::getLeadershipGeneration() const
	{
		float modifier = 1.0f;
		if (controller != owner)
			modifier *= bEnd::Politics::getPolitics(controller).getOccupationPolicy(owner).getLeadershipModifier();
		else if (!hasCore(controller))
			modifier *= ANNEXED_NON_CORE_PENALTY;

		return leadershipGeneration * modifier;
	}

	const float Region::getManpowerGeneration() const
	{
		float modifier = 1.0f;
		if (controller != owner)
			modifier *= bEnd::Politics::getPolitics(controller).getOccupationPolicy(owner).getMPModifier();
		else if (!hasCore(controller))
			modifier *= ANNEXED_NON_CORE_PENALTY;

		return manpowerGeneration * modifier;
	}

	const float Region::getResourceGeneration(const Resource resourceType) const
	{
		float modifier = 1.0f;
		if (controller != owner)
			modifier *= bEnd::Politics::getPolitics(controller).getOccupationPolicy(owner).getResourceModifier();
		else if (!hasCore(controller))
			modifier *= ANNEXED_NON_CORE_PENALTY;

		return resourceGeneration[resourceType] * modifier;
	}

	const float Region::getIC() const
	{
		float modifier = 1.0f;
		if (controller != owner)
			modifier *= bEnd::Politics::getPolitics(controller).getOccupationPolicy(owner).getICModifier();
		else if (!hasCore(controller))
			modifier *= ANNEXED_NON_CORE_PENALTY;

		return buildings["IC"].first * IC_POINTS_PER_LEVEL * modifier;
	}

	bool Region::hasCore(const Tag& tag)const
	{
		return bool(cores.count(tag));
	}
}
