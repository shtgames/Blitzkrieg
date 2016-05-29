#include "Unit.h"
#include "FileProcessor.h"

#include "Region.h"

#include "Politics.h"
#include "ResourceDistributor.h"
#include "LeadershipDistributor.h"

#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <stack>

using namespace std;

namespace bEnd
{
	map<unsigned short, Region> Region::regions;
	const float Region::ANNEXED_NON_CORE_PENALTY = 0.7f;

	Region::Region()
		: IC(std::make_pair(0.0f, 1.0f)), leadership(std::make_pair(0.0f, 1.0f)), manpowerGeneration(std::make_pair(0.0f, 1.0f)), generatingResources(false)
	{
		for (unsigned char it(0); it < Resource::Last; it++)
			resourceGeneration[(Resource)it] = make_pair(0.0f, 1.0f);
	}

	void Region::changeOwner(const Tag& tag)
	{
		if (tag == owner) return;
		if ((tag != controller && owner == controller) ||
			(tag == controller && owner != controller))
		{
			stopGeneratingResources();
			owner = tag;
			generateResources();
		}
		else owner = tag;
	}

	void Region::changeController(const Tag& tag)
	{
		if (tag == controller) return;

		stopGeneratingResources();
		controller = tag;
		generateResources();
	}

	void Region::generateResources()
	{
		if (generatingResources) return;

		ResourceDistributor& resourceDistributor = ResourceDistributor::get(controller);
		resourceDistributor.changeICAmount(getIC());
		resourceDistributor.changeManpowerGain(getManpowerGeneration());

		resourceLock.lock();
		for (auto it = 0; it < Resource::Last; it++)
			resourceDistributor.changeResourceGain((Resource)it, getResourceGeneration((Resource)it), ResourceDistributor::Generated);
		resourceLock.unlock();

		LeadershipDistributor::get(controller).changeLeadershipAmount(getLeadership());

		generatingResources = true;
	}

	void Region::stopGeneratingResources()
	{
		if (!generatingResources) return;

		ResourceDistributor& resourceDistributor = ResourceDistributor::get(controller);
		resourceDistributor.changeICAmount((-1) * getIC());
		resourceDistributor.changeManpowerGain((-1) * getManpowerGeneration());

		resourceLock.lock();
		for (unsigned char it = 0; it < Resource::Last; it++)
			resourceDistributor.changeResourceGain((Resource)it, (-1) * getResourceGeneration((Resource)it), ResourceDistributor::Generated);
		resourceLock.unlock();

		LeadershipDistributor::get(controller).changeLeadershipAmount((-1) * getLeadership());

		generatingResources = false;
	}

	void Region::repair(const Unit& building, float levels)
	{
		if (building.getType() != Unit::Building) return;

		BuildingLevels& buffer = buildings[building.getName()];

		if (buffer.first + levels > buffer.second)
			levels = buffer.second - buffer.first;
		else if (levels < buffer.first) levels = buffer.first;

		if (levels == 0.0f) return;

		stopGeneratingResources();
		resourceLock.lock();
		for (unsigned char it(0); it < Resource::Last; it++)
		{
			resourceGeneration[(Resource)it].first = resourceGeneration[(Resource)it].first +
				building.getResourceAddition((Resource)it) * levels;
			resourceGeneration[(Resource)it].second = resourceGeneration[(Resource)it].second *
				pow(building.getResourceMultiplier((Resource)it), levels);
		}
		resourceLock.unlock();

		IC.first = IC.first + building.getICAddition() * levels;
		IC.second = IC.second * pow(building.getICMultiplier(), levels);

		leadership.first = leadership.first + building.getLeadershipAddition() * levels;
		leadership.second = leadership.second * pow(building.getLeadershipMultiplier(), levels);

		manpowerGeneration.first = manpowerGeneration.first + building.getManpowerAddition() * levels;
		manpowerGeneration.second = manpowerGeneration.second * pow(building.getManpowerMultiplier(), levels);

		generateResources();

		buffer.first += levels;
	}

	void Region::addCore(const Tag& tag)
	{
		coresLock.lock();
		if (cores.count(tag))
		{
			coresLock.unlock();
			return;
		}
		cores.emplace(tag);
		coresLock.unlock();

		stopGeneratingResources();
		generateResources();
	}

	void Region::enqueueBuilding(const std::string& key, const unsigned char amount)
	{
		std::lock_guard<std::mutex> guard(queueLock);
		queuedBuildingCount[key] += amount;
		if (queuedBuildingCount.at(key) > 10)
			queuedBuildingCount.at(key) = 10;
	}

	void Region::dequeueBuilding(const std::string& key, unsigned char amount)
	{
		std::lock_guard<std::mutex> guard(queueLock);
		if (queuedBuildingCount[key] < amount)
			amount = queuedBuildingCount.at(key);
		queuedBuildingCount.at(key) -= amount;
	}

	const unsigned char Region::getQueuedCount(const std::string & key)
	{
		std::lock_guard<std::mutex> guard(queueLock);
		return queuedBuildingCount[key];
	}

	void Region::build(const Unit& building)
	{
		if (building.getType() != Unit::Building) return;

		BuildingLevels& buffer = buildings[building.getName()];
		if (buffer.second <= 9)
		{
			stopGeneratingResources();
			resourceLock.lock();
			for (unsigned char it(0); it < Resource::Last; it++)
			{
				resourceGeneration.at((Resource)it).first = resourceGeneration.at((Resource)it).first + building.getResourceAddition((Resource)it);
				resourceGeneration.at((Resource)it).second = resourceGeneration.at((Resource)it).second * building.getResourceMultiplier((Resource)it);
			}
			resourceLock.unlock();

			IC.first = IC.first + building.getICAddition();
			IC.second = IC.second * building.getICMultiplier();

			leadership.first = leadership.first + building.getLeadershipAddition();
			leadership.second = leadership.second * building.getLeadershipMultiplier();

			manpowerGeneration.first = manpowerGeneration.first + building.getManpowerAddition();
			manpowerGeneration.second = manpowerGeneration.second * building.getManpowerMultiplier();

			generateResources();

			buffer.first++;
			buffer.second++;
		}

		dequeueBuilding(building.getName());
	}

	void Region::repairAll()
	{
		///
	}

	void Region::loadFromSave(const FileProcessor::Statement& source)
	{
		Region& target(regions[std::stoi(source.lValue)]);

		for (const auto& it : source.rStatements)
		{
			if (it.lValue == "owner") target.changeOwner(it.rStrings.at(0));
			else if (it.lValue == "controller") target.changeController(it.rStrings.at(0));
			else if (it.lValue == "core") target.addCore(it.rStrings.at(0));
			else if (it.lValue == "manpower") target.manpowerGeneration.first = std::stof(it.rStrings.at(0));
			else if (it.lValue == "leadership") target.leadership.first = std::stof(it.rStrings.at(0));
			else if (it.lValue == "points") target.victoryPoints = std::stof(it.rStrings.at(0));
			else if (it.lValue == "capital" && it.rStrings.at(0) == "yes") target.capital = true;
			else if (it.lValue == "pool")
				for (const auto& it1 : it.rStatements)
				{
					if (it1.lValue == "energy") ResourceDistributor::get(target.controller).changeResourceAmount(Energy, std::stof(it1.rStrings.at(0)));
					else if (it1.lValue == "metal") ResourceDistributor::get(target.controller).changeResourceAmount(Metal, std::stof(it1.rStrings.at(0)));
					else if (it1.lValue == "rare_materials") ResourceDistributor::get(target.controller).changeResourceAmount(RareMaterials, std::stof(it1.rStrings.at(0)));
					else if (it1.lValue == "crude_oil") ResourceDistributor::get(target.controller).changeResourceAmount(CrudeOil, std::stof(it1.rStrings.at(0)));
					else if (it1.lValue == "supplies") ResourceDistributor::get(target.controller).changeResourceAmount(Supplies, std::stof(it1.rStrings.at(0)));
					else if (it1.lValue == "money") ResourceDistributor::get(target.controller).changeResourceAmount(Money, std::stof(it1.rStrings.at(0)));
					else if (it1.lValue == "fuel") ResourceDistributor::get(target.controller).changeResourceAmount(Fuel, std::stof(it1.rStrings.at(0)));
				}
			else if (it.lValue == "max_producing")
				for (const auto& it1 : it.rStatements)
				{
					if (it1.lValue == "energy")	target.resourceGeneration[Energy].first = target.manpowerGeneration.first = std::stof(it1.rStrings.at(0));
					else if (it1.lValue == "metal") target.resourceGeneration[Metal].first = target.manpowerGeneration.first = std::stof(it1.rStrings.at(0));
					else if (it1.lValue == "rare_materials") target.resourceGeneration[RareMaterials].first = target.manpowerGeneration.first = std::stof(it1.rStrings.at(0));
					else if (it1.lValue == "crude_oil") target.resourceGeneration[CrudeOil].first = target.manpowerGeneration.first = std::stof(it1.rStrings.at(0));
				}
			else if (Unit::exists(it.lValue) && Unit::get(it.lValue).getType() == Unit::Building)
			{
				target.buildings[it.lValue].second = std::stof(it.rStrings.at(0));
				target.repair(Unit::get(it.lValue), std::stof(it.rStrings.at(1)));
			}
		}
		target.generatingResources = false;
		target.generateResources();
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

	const Region::BuildingLevels Region::getBuildingLevels(const std::string& key)
	{
		if (buildings.count(key))
			return buildings.at(key);
		else return BuildingLevels(0, 0);
	}

	const bool Region::isSea() const
	{
		return sea;
	}

	bool Region::hasCore(const Tag& tag)const
	{
		std::lock_guard<std::mutex> guard(coresLock);
		return cores.count(tag);
	}
}
