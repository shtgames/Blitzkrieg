#include "Unit.h"

#include "Region.h"

#include "Politics.h"
#include "ResourceDistributor.h"
#include "LeadershipDistributor.h"
#include "Unit.h"

#include <fstream>
#include <string>

const std::string CACHEDIR = "map/cache/provinces.bin";

using namespace std;

namespace bEnd
{
	map<unsigned short, Region> Region::regions;
	const float Region::ANNEXED_NON_CORE_PENALTY = 0.7f;

	Region::Region()
	{
		IC = std::make_pair(0.0f, 1.0f);
		leadership = std::make_pair(0.0f, 1.0f);
		manpowerGeneration = std::make_pair(0.0f, 1.0f);
		for (unsigned char it = 0; it < Resource::Last; it++)
			resourceGeneration[Resource(it)] = make_pair(0.0f, 1.0f);
	}

	void Region::changeOwner(const Tag& tag)
	{
		stopGeneratingResources();
		owner = tag;
		addCore(owner);
		generateResources();
	}

	void Region::changeController(const Tag& tag)
	{
		stopGeneratingResources();
		controller = tag;
		generateResources();
	}

	void Region::generateResources()
	{
		ResourceDistributor& resourceDistributor = ResourceDistributor::get(controller);
		resourceDistributor.changeICAmount(getIC());
		resourceDistributor.changeManpowerGain(getManpowerGeneration());
		for (unsigned char it = 0; it < Resource::Last; it++)
			resourceDistributor.changeResourceGain(Resource(it), getResourceGeneration(Resource(it)));

		LeadershipDistributor::get(controller).changeLeadershipAmount(getLeadership());
	}

	void Region::stopGeneratingResources()
	{
		ResourceDistributor& resourceDistributor = ResourceDistributor::get(controller);
		resourceDistributor.changeICAmount((-1) * getIC());
		resourceDistributor.changeManpowerGain((-1) * getManpowerGeneration());
		for (unsigned char it = 0; it < Resource::Last; it++)
			resourceDistributor.changeResourceGain(Resource(it), (-1) * getResourceGeneration(Resource(it)));

		LeadershipDistributor::get(controller).changeLeadershipAmount((-1) * getLeadership());
	}

	void Region::addCore(const Tag& tag)
	{
		stopGeneratingResources();
		cores.emplace(tag);
		generateResources();
	}

	void Region::build(const Unit& building)
	{
		if (building.getType() != Unit::Building) return;

		BuildingLevels& buffer = buildings[building.getName()];
		if (buffer.second <= 9.0f)
		{
			stopGeneratingResources();

			IC.first += building.getICAddition();
			IC.second += building.getICMultiplier();

			leadership.first += building.getLeadershipAddition();
			leadership.second += building.getLeadershipMultiplier();

			manpowerGeneration.first += building.getManpowerAddition();
			manpowerGeneration.second += building.getManpowerMultiplier();

			for (auto it = resourceGeneration.begin(), end = resourceGeneration.end(); it != end; ++it)
			{
				it->second.first += building.getResourceAddition(it->first);
				it->second.second += building.getResourceMultiplier(it->first);
			}

			generateResources();

			buffer.first++;
			buffer.second++;
		}
	}

	const bool Region::loadFromFile(ifstream& file)
	{
		return false;
	}
	
	const float Region::getLeadership() const
	{
		float modifier = 1.0f;
		if (controller != owner)
			modifier *= bEnd::Politics::get(controller).getOccupationPolicy(owner).getLeadershipModifier();
		else if (!hasCore(controller))
			modifier *= ANNEXED_NON_CORE_PENALTY;

		return leadership.first * leadership.second * modifier;
	}

	const float Region::getManpowerGeneration() const
	{
		float modifier = 1.0f;
		if (controller != owner)
			modifier *= bEnd::Politics::get(controller).getOccupationPolicy(owner).getMPModifier();
		else if (!hasCore(controller))
			modifier *= ANNEXED_NON_CORE_PENALTY;

		return manpowerGeneration.first * manpowerGeneration.second * modifier;
	}

	const float Region::getResourceGeneration(const Resource resourceType) const
	{
		float modifier = 1.0f;
		if (controller != owner)
			modifier *= bEnd::Politics::get(controller).getOccupationPolicy(owner).getResourceModifier();
		else if (!hasCore(controller))
			modifier *= ANNEXED_NON_CORE_PENALTY;

		return resourceGeneration[resourceType].first * resourceGeneration[resourceType].second * modifier;
	}

	const float Region::getIC() const
	{
		float modifier = 1.0f;
		if (controller != owner)
			modifier *= bEnd::Politics::get(controller).getOccupationPolicy(owner).getICModifier();
		else if (!hasCore(controller))
			modifier *= ANNEXED_NON_CORE_PENALTY;

		return IC.first * IC.second * modifier;
	}

	bool Region::hasCore(const Tag& tag)const
	{
		return bool(cores.count(tag));
	}
}
