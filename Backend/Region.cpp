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
		: IC(std::make_pair(0.0f, 1.0f)), leadership(std::make_pair(0.0f, 1.0f)), manpowerGeneration(std::make_pair(0.0f, 1.0f))
	{
		for (unsigned char it = 0; it < Resource::Last; it++)
			resourceGeneration[Resource(it)] = make_pair(0.0f, 1.0f);
	}

	void Region::changeOwner(const Tag& tag)
	{
		owner = tag;
		addCore(owner);
	}

	void Region::changeController(const Tag& tag)
	{
		controller = tag;

		resourceLock.lock();
		stopGeneratingResources();
		generateResources();
		resourceLock.unlock();
	}

	void Region::generateResources()
	{
		ResourceDistributor& resourceDistributor = ResourceDistributor::get(controller);
		resourceDistributor.changeICAmount(getIC());
		resourceDistributor.changeManpowerGain(getManpowerGeneration());
		for (unsigned char it = 0; it < Resource::Last; it++)
			resourceDistributor.changeResourceGain((Resource)it, getResourceGeneration((Resource)it));

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
		coresLock.lock();
		cores.emplace(tag);
		coresLock.unlock();

		resourceLock.lock();
		stopGeneratingResources();
		generateResources();
		resourceLock.unlock();
	}

	void Region::build(const Unit& building)
	{
		if (building.getType() != Unit::Building) return;

		BuildingLevels& buffer = buildings[building.getName()];
		if (buffer.second <= 9.0f)
		{
			IC.first = IC.first + building.getICAddition();
			IC.second = IC.second + building.getICMultiplier();

			leadership.first = leadership.first + building.getLeadershipAddition();
			leadership.second = leadership.second + building.getLeadershipMultiplier();

			manpowerGeneration.first = manpowerGeneration.first + building.getManpowerAddition();
			manpowerGeneration.second = manpowerGeneration.second + building.getManpowerMultiplier();

			resourceLock.lock();
			stopGeneratingResources();
			for (unsigned char it = 0; it < Resource::Last; it++)
			{
				resourceGeneration.at((Resource)it).first = resourceGeneration.at((Resource)it).first + building.getResourceAddition((Resource)it);
				resourceGeneration.at((Resource)it).second = resourceGeneration.at((Resource)it).second + building.getResourceMultiplier((Resource)it);
			}
			generateResources();
			resourceLock.unlock();

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
			modifier *= OccupationPolicy::get(Politics::get(controller).getOccupationPolicy(owner)).getLeadershipModifier();
		else if (!hasCore(controller))
			modifier *= ANNEXED_NON_CORE_PENALTY;

		return leadership.first * leadership.second * modifier;
	}

	const float Region::getManpowerGeneration() const
	{
		float modifier = 1.0f;
		if (controller != owner)
			modifier *= OccupationPolicy::get(Politics::get(controller).getOccupationPolicy(owner)).getMPModifier();
		else if (!hasCore(controller))
			modifier *= ANNEXED_NON_CORE_PENALTY;

		return manpowerGeneration.first * manpowerGeneration.second * modifier;
	}

	const float Region::getResourceGeneration(const Resource resourceType) const
	{
		float modifier = 1.0f;
		if (controller != owner)
			modifier *= OccupationPolicy::get(Politics::get(controller).getOccupationPolicy(owner)).getResourceModifier();
		else if (!hasCore(controller))
			modifier *= ANNEXED_NON_CORE_PENALTY;

		return resourceGeneration.at(resourceType).first * resourceGeneration.at(resourceType).second * modifier;
	}

	const float Region::getIC() const
	{
		float modifier = 1.0f;
		if (controller != owner)
			modifier *= OccupationPolicy::get(Politics::get(controller).getOccupationPolicy(owner)).getICModifier();
		else if (!hasCore(controller))
			modifier *= ANNEXED_NON_CORE_PENALTY;

		return IC.first * IC.second * modifier;
	}

	bool Region::hasCore(const Tag& tag)const
	{
		std::lock_guard<std::mutex> guard(coresLock);
		return cores.count(tag);
	}
}
