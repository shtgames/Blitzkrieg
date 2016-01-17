#include "Region.h"

#include "Nation.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <math.h>
#include <thread>

const std::string CACHEDIR = "map/cache/provinces.bin";

namespace bEnd
{
	Region::Construction::Construction(const Structure _building, const unsigned short _Region) : buildingType(_building), targetRegion(_Region)
	{
		regions[targetRegion].buildings[buildingType].second++;
	}

	void Region::Construction::onCompletion()
	{
		regions[targetRegion].buildings[buildingType].second--;
		regions[targetRegion].buildings[buildingType].first.second++;
		regions[targetRegion].buildings[buildingType].first.first++;
	}


	void Region::generateResourcesGlobal(std::map<Tag, Nation>& nations)
	{
		for (auto it = regions.begin(), end = regions.end(); it != end; ++it)
			it->second.generateResources(nations);
	}

	void Region::generateResources(std::map<Tag, Nation>& nations)
	{
		const bool temp = hasCore(controller);
		Nation& buffer = nations.at(controller);
		buffer.production.transferResourcesFromRegion(resourceGeneration, owner == controller, temp, buffer.politics.getOccupationPolicy(owner));
		buffer.production.transferManpowerFromRegion(manpowerGeneration, owner == controller, temp, buffer.politics.getOccupationPolicy(owner));
		buffer.production.transferIC(buildings[Industry].first.first, owner == controller, temp, buffer.politics.getOccupationPolicy(owner));
		buffer.technology.transferLeadership(leadershipGeneration, owner == controller, temp, buffer.politics.getOccupationPolicy(owner));
	}

	void Region::changeOwner(const Tag& _newOwner)
	{
		owner = _newOwner;
		addCore(owner);
	}

	void Region::changeController(const Tag& _newController)
	{
		controller = _newController;
	}

	void Region::addCore(const Tag& _newCore)
	{
		cores.emplace(_newCore);
	}

	bool Region::hasCore(const Tag& _Tag)const
	{
		return bool(cores.count(_Tag));
	}

	void Region::build(const Structure _building)
	{
		Nation::nations[controller].production.addProductionItem(&Construction(_building, provID));
	}

}
