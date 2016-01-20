#include "Region.h"

#include "Politics.h"

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
	const float Region::ANNEXED_NON_CORE_PENALTY = 0.7f, Region::IC_POINTS_PER_LEVEL = 1.5f;

	Region::Construction::Construction(const BuildingType _building, const unsigned short _Region) : buildingType(_building), targetRegion(_Region)
	{
		regions[targetRegion].buildings[buildingType].second++;
	}

	Region::Construction::~Construction()
	{
		regions[targetRegion].buildings[buildingType].second--;
	}

	void Region::Construction::onCompletion()
	{
		regions[targetRegion].buildings[buildingType].first.second++;
		regions[targetRegion].buildings[buildingType].first.first++;
	}

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

		return buildings[Industry].first.first * IC_POINTS_PER_LEVEL * modifier;
	}

	bool Region::hasCore(const Tag& tag)const
	{
		return bool(cores.count(tag));
	}
}
