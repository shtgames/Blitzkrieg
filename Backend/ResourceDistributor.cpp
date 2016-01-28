#include "Region.h"

#include "ResourceDistributor.h"

#include "Production.h"

namespace bEnd
{
	unordered_map<Tag, unique_ptr<ResourceDistributor>> ResourceDistributor::resourceDistributors;
	const float ResourceDistributor::BASE_IC = 1.0f;


	ResourceDistributor::ResourceDistributor()
		: baseIC(BASE_IC), availableIC(BASE_IC), wastedIC(0)
	{
		ICDistribution[ProductionLine].first = 1.0f;
	}

	const bool ResourceDistributor::contains(const std::map<Resource, float>& _rVal)const
	{
		for (auto it = _rVal.begin(), end = _rVal.end(); it != end; ++it)
			if (resources.at(it->first).first < it->second) return false;
		return true;
	}

	void ResourceDistributor::transferResourcesFromTrade(const std::map<Resource, float>& _source)
	{
		for (auto it = _source.begin(), end = _source.end(); it != end; ++it)
			if (resources[it->first].first + it->second > 0.0f)
			{
				resources[it->first].first += it->second;
				resources[it->first].second[traded] += it->second;
			}
	}

	void ResourceDistributor::transferResourcesFromRegion(const Region& region)
	{
		resources[Energy].first += region.getResourceGeneration(Energy); // * bonuses;
		resources[Energy].second[generated] += region.getResourceGeneration(Energy); // * bonuses;

		resources[Metal].first += region.getResourceGeneration(Metal); // * bonuses;
		resources[Metal].second[generated] += region.getResourceGeneration(Metal); // * bonuses;

		resources[RareMaterials].first += region.getResourceGeneration(RareMaterials); // * bonuses;
		resources[RareMaterials].second[generated] += region.getResourceGeneration(RareMaterials); // * bonuses;

		resources[CrudeOil].first += region.getResourceGeneration(CrudeOil); // * bonuses;
		resources[CrudeOil].second[generated] += region.getResourceGeneration(CrudeOil); // * bonuses;
	}

	void ResourceDistributor::transferManpowerFromRegion(const Region& region)
	{
		manpower += region.getManpowerGeneration(); // * bonuses;
	}

	void ResourceDistributor::transferICFromRegion(const Region& region)
	{
		baseIC += region.getIC();
	}

	void ResourceDistributor::resetIncome()
	{
		baseIC = BASE_IC;
		availableIC = BASE_IC; // * bonuses;
		wastedIC = 0;

		for (auto it = resources.begin(), end = resources.end(); it != end; ++it)
			for (auto it1 = it->second.second.begin(), end1 = it->second.second.end(); it1 != end1; ++it1)
				it1->second = 0.0f;
	}

	void ResourceDistributor::update()
	{
		calculateIC();
		resourceConversions();
		distributeIC();
		calculateMoney();

		Production::getProduction(tag).update();
		// Upgrades::...
	}

	void ResourceDistributor::setICDistributionValue(const ICDistributionCategory category, const double factor)
	{
		ICDistribution[category].second = false;
		const double difference = (factor >= 0.0f ? factor : 0.0f) - ICDistribution[category].first;

		if(difference > 0.0f) do
		{
			double min = 1.0f;
			unsigned char unlockedCategoryCount = 0;

			for (auto it = ICDistribution.begin(), end = ICDistribution.end(); it != end; ++it)
			{
				if (it->first == category) continue;
				if (!it->second.second && !(difference > 0.0f && it->second.first == 0.0f))
				{
					unlockedCategoryCount++;
					if (it->second.first < min) min = it->second.first;
				}
			}
			if (unlockedCategoryCount == 0) break;

			const double changeAmount = difference / unlockedCategoryCount > min ? min : difference / unlockedCategoryCount;
			for (auto it = ICDistribution.begin(), end = ICDistribution.end(); it != end; ++it)
				if(it->first != category) it->second.first -= changeAmount;

			ICDistribution[category].first += changeAmount * unlockedCategoryCount;

		} while (ICDistribution[category].first != factor);
		else
		{
			unsigned char unlockedCategoryCount = 0;
			for (auto it = ICDistribution.begin(), end = ICDistribution.end(); it != end; ++it)
			{
				if (it->first == category) continue;
				if (!it->second.second) unlockedCategoryCount++;
			}

			const double changeAmount = difference / unlockedCategoryCount;
			for (auto it = ICDistribution.begin(), end = ICDistribution.end(); it != end; ++it)
				if (it->first != category) it->second.first -= changeAmount;

			ICDistribution[category].first += changeAmount * unlockedCategoryCount;
		}
	}

	void ResourceDistributor::setICDistributionValueLock(const ICDistributionCategory category, const bool lock)
	{
		ICDistribution[category].second = lock;
	}

	const bool ResourceDistributor::loadFromFile(ifstream& file)
	{
		return false;
	}

	void ResourceDistributor::calculateIC()
	{
		float usedIC = 0.0f;

		for (auto it = ICDistribution.begin(), end = ICDistribution.end(); it != end; ++it)
			if (it->second.first > ICNeeds[it->first]) { wastedIC += it->second.first - ICNeeds[it->first]; usedIC += ICNeeds[it->first]; }
			else usedIC += it->second.first;

		if (baseIC > 0.0f)
		{
			float resourceBottleneck = 1.0f;
			if (resources[Energy].first < (baseIC * 2.0f)) // 1 x IC = 2 x Energy + 1 x Metal + 0.5 x Rare Materials;
				resourceBottleneck = (resources[Energy].second[generated] + resources[Energy].second[traded]) / (baseIC * 2.0f);

			if (resources[Metal].first < (baseIC * 1.0f))
				if (((resources[Metal].second[generated] + resources[Metal].second[traded]) / (baseIC * 1.0f)) < resourceBottleneck)
					resourceBottleneck = (resources[Metal].second[generated] + resources[Metal].second[traded]) / (baseIC * 1.0f);

			if (resources[RareMaterials].first < (baseIC * 0.5f))
				if (((resources[RareMaterials].second[generated] + resources[RareMaterials].second[traded]) / (baseIC * 0.5f)) < resourceBottleneck)
					resourceBottleneck = ((resources[RareMaterials].second[generated] + resources[RareMaterials].second[traded]) / (baseIC * 0.5f));

			availableIC = resourceBottleneck * baseIC; // * bonuses;
			
			resources[Energy].first -= (usedIC > availableIC ? availableIC : usedIC) * 2.0f;
			resources[Energy].second[used] = (usedIC > availableIC ? availableIC : usedIC) * (-2.0f);

			resources[Metal].first -= (usedIC > availableIC ? availableIC : usedIC) * 1.0f;
			resources[Metal].second[used] = (usedIC > availableIC ? availableIC : usedIC) * (-1.0f);

			resources[RareMaterials].first -= (usedIC > availableIC ? availableIC : usedIC) * 0.5f;
			resources[RareMaterials].second[used] = (usedIC > availableIC ? availableIC : usedIC) * (-0.5f);
		}
	}

	void ResourceDistributor::resourceConversions()
	{
		if (availableIC > 0.0f)
		{
			float resourceBottleneck = 1.0f;
			if (resources[Energy].first < 0.5f * availableIC) //The Amount of Energy Converted to Crude Oil = (10 x Available IC / 20) / The level of “Coal to Oil Conversion” Technology
				resourceBottleneck = resources[Energy].first / (0.5f * availableIC);

			resources[Energy].first -= resourceBottleneck * (0.5f * availableIC);
			resources[Energy].second[convertedIntoCrudeOil] = resourceBottleneck * (-0.5f * availableIC);

			resources[CrudeOil].first += resourceBottleneck * (availableIC / 20.0f);
			resources[CrudeOil].second[convertedFromEnergy] = resourceBottleneck * (availableIC / 20.0f);

			resourceBottleneck = 1.0f;

			if (resources[CrudeOil].first < availableIC / 2.0f) //Total Fuel Production = (Base IC / 2) x(1 + (PercenTage of The “Oil Refining” Technology - 0, 1))
				resourceBottleneck = resources[CrudeOil].first / (availableIC / 2.0f);

			resources[CrudeOil].first -= resourceBottleneck * (availableIC / 2.0f);
			resources[CrudeOil].second[convertedIntoFuel] = resourceBottleneck * (availableIC / -2.0f);

			resources[Fuel].first += resourceBottleneck * (availableIC / 2.0f);
			resources[Fuel].second[convertedFromCrudeOil] = resourceBottleneck * (availableIC / 2.0f);
		}
	}


	void ResourceDistributor::distributeIC()
	{
		wastedIC += Production::getProduction(tag).setIC(availableIC * ICDistribution[ProductionLine].first);
		// wastedIC += Upgrades::...
	}

	void ResourceDistributor::calculateMoney()
	{
		resources[Money].first += (baseIC - wastedIC) / 20.0f; // Money Gain = (Base IC / 20) x ( 1 + ( "Economic Laws" PercenTage of Money + "Education Investment Laws" PercenTage of Money + "Minister traits" PercenTage of Money + “Event” PercenTage of Money ) )
		resources[Money].second[generated] = (baseIC - wastedIC) / 20.0f;
	}	
}