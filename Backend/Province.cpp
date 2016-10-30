#include "Unit.h"
#include "FileProcessor.h"

#include "Province.h"

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
	map<unsigned short, Province> Province::provinces;
	const float Province::ANNEXED_NON_CORE_PENALTY = 0.7f;

	Province::Province()
	{
		leadership.first = IC.first = manpowerGeneration.first = { 0.0f };
		leadership.second = IC.second = manpowerGeneration.second = { 1.0f };

		for (unsigned char it(0); it < Resource::Last; it++)
			resourceGeneration[(Resource)it] = make_pair(0.0f, 1.0f);
	}

	void Province::changeOwner(const Tag& tag)
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

	void Province::changeController(const Tag& tag)
	{
		if (tag == controller) return;

		stopGeneratingResources();
		controller = tag;
		generateResources();
	}

	void Province::generateResources()
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

	void Province::stopGeneratingResources()
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

	void Province::reset()
	{
		victoryPoints = 0;
		capital = false;
		generatingResources = false;
		IC = std::make_pair(0.0f, 1.0f);
		leadership = std::make_pair(0.0f, 1.0f);
		manpowerGeneration = std::make_pair(0.0f, 1.0f);

		for (auto& it : buildings) it.second = std::make_pair(0, 0);

		resourceLock.lock();
		for (unsigned char it(0); it < Resource::Last; it++)
			resourceGeneration[(Resource)it] = make_pair(0.0f, 1.0f);
		resourceLock.unlock();

		coresLock.lock();
		cores.clear();
		coresLock.unlock();

		queuedBuildingCount.clear();
	}

	void Province::repair(const Unit& building, float levels)
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

	void Province::addCore(const Tag& tag)
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

	void Province::enqueueBuilding(const std::string& key, const unsigned char amount)
	{
		std::lock_guard<std::mutex> guard(queueLock);
		queuedBuildingCount[key] += amount;
		if (queuedBuildingCount.at(key) > 10)
			queuedBuildingCount.at(key) = 10;
	}

	void Province::dequeueBuilding(const std::string& key, unsigned char amount)
	{
		std::lock_guard<std::mutex> guard(queueLock);
		if (queuedBuildingCount[key] < amount)
			amount = queuedBuildingCount.at(key);
		queuedBuildingCount.at(key) -= amount;
	}

	const unsigned char Province::getQueuedCount(const std::string & key)
	{
		std::lock_guard<std::mutex> guard(queueLock);
		return queuedBuildingCount[key];
	}

	void Province::build(const Unit& building)
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

	void Province::repairAll()
	{
		///
	}

	const std::unordered_map<unsigned short, unsigned short>& Province::getNeighbours() const
	{
		return neighbours;
	}
	
	const std::set<unsigned short> Province::getPath(const unsigned short target) const
	{
		std::set<unsigned short> returnValue;
		if (!provinces.count(target)) return returnValue;
		return returnValue;
	}

	void Province::loadFromSave(const FileProcessor::Statement& source)
	{
		const auto ID(std::stoi(source.lValue));
		if (provinces.count(ID)) provinces.at(ID).reset();
		else provinces[ID];
		Province& target(provinces.at(ID));

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
	
	const float Province::getLeadership() const
	{
		float modifier = 1.0f;
		if (controller != owner)
			modifier *= OccupationPolicy::get(Politics::get(controller).getOccupationPolicy(owner)).getLeadershipModifier();
		else if (!hasCore(controller))
			modifier *= ANNEXED_NON_CORE_PENALTY;

		return leadership.first * leadership.second * modifier;
	}

	const float Province::getManpowerGeneration() const
	{
		float modifier = 1.0f;
		if (controller != owner)
			modifier *= OccupationPolicy::get(Politics::get(controller).getOccupationPolicy(owner)).getMPModifier();
		else if (!hasCore(controller))
			modifier *= ANNEXED_NON_CORE_PENALTY;

		return manpowerGeneration.first * manpowerGeneration.second * modifier;
	}

	const float Province::getResourceGeneration(const Resource resourceType) const
	{
		float modifier = 1.0f;
		if (controller != owner)
			modifier *= OccupationPolicy::get(Politics::get(controller).getOccupationPolicy(owner)).getResourceModifier();
		else if (!hasCore(controller))
			modifier *= ANNEXED_NON_CORE_PENALTY;
		
		return resourceGeneration.at(resourceType).first * resourceGeneration.at(resourceType).second * modifier;
	}

	const float Province::getIC() const
	{
		float modifier = 1.0f;
		if (controller != owner)
			modifier *= OccupationPolicy::get(Politics::get(controller).getOccupationPolicy(owner)).getICModifier();
		else if (!hasCore(controller))
			modifier *= ANNEXED_NON_CORE_PENALTY;

		return IC.first * IC.second * modifier;
	}

	const Province::BuildingLevels Province::getBuildingLevels(const std::string& key)
	{
		if (buildings.count(key))
			return buildings.at(key);
		else return BuildingLevels(0, 0);
	}

	const bool Province::isSea() const
	{
		return sea;
	}

	const bool Province::isCoastal() const
	{
		return coastal;
	}

	bool Province::hasCore(const Tag& tag)const
	{
		std::lock_guard<std::mutex> guard(coresLock);
		return cores.count(tag);
	}
}
