#include "Province.h"

#include "ResourceDistributor.h"

#include "LeadershipDistributor.h"
#include "Production.h"

namespace bEnd
{
	unordered_map<Tag, unique_ptr<ResourceDistributor>> ResourceDistributor::resourceDistributors;
	const float ResourceDistributor::BASE_IC = 2.0f, ResourceDistributor::ENERGY_PER_IC_POINT = 2.0f, ResourceDistributor::METAL_PER_IC_POINT = 1.0f, ResourceDistributor::RARE_MATERIALS_PER_IC_POINT = 0.5f;
	
	ResourceDistributor::ResourceDistributor(const Tag& tag)
		: tag(tag)
	{
		manpower.second = std::make_pair(0.0f, 1.0f);

		for (auto it(0); it <= ICDistributionCategory::ToLendLease; it++)
			ICDistribution[(ICDistributionCategory)it] = std::make_pair(0.0f, false);
		ICDistribution[ToProductionLine] = std::make_pair(1.0f, false);
	}

	const bool ResourceDistributor::contains(const std::map<Resource, float>& _rVal)const
	{
		std::lock_guard<std::mutex> guard(resourcesLock);
		for (auto it = _rVal.begin(), end = _rVal.end(); it != end; ++it)
			if (resources.at(it->first).amount < it->second) return false;
		return true;
	}

	void ResourceDistributor::update()
	{
		resourcesLock.lock();

		for (auto& it : resources) it.second.resetTotal();

		useResourcesForIC();
		resourceConversions();
		distributeIC();
		calculateMoneyChangeAmount();

		for (auto it(0); it < Resource::Last; it++) resources[(Resource)it].apply();

		resourcesLock.unlock();

		manpower.first = manpower.first + (manpower.second.first * manpower.second.second) / 30;

		Production::get(tag).update();
		// Upgrades::...
		// ...
	}

	void ResourceDistributor::reset()
	{
		for (auto& it : resourceDistributors)
		{
			Production::get(it.first).reset();
			LeadershipDistributor::get(it.first).reset();

			it.second->wastedIC = 0.0f;
			it.second->ICResourceBottleneck = 1.0f;
			it.second->IC = make_pair(BASE_IC, 1.0f);
			it.second->manpower.first = 0;
			it.second->manpower.second = std::make_pair(0.0f, 1.0f);

			it.second->ICDistributionLock.lock();
			for (auto it1(0); it1 <= ICDistributionCategory::ToLendLease; it1++)
				it.second->ICDistribution[(ICDistributionCategory)it1] = std::make_pair(0.0f, false);
			it.second->ICDistribution[ToProductionLine] = std::make_pair(1.0f, false);
			it.second->ICDistributionLock.unlock();
		}
	}

	void ResourceDistributor::setICDistributionValue(const ICDistributionCategory category, const double factor)
	{
		std::lock_guard<std::mutex> guard(ICDistributionLock);

		ICDistribution[category].second = false;

		std::vector<decltype(ICDistribution.begin())> sources;
		const double difference((factor >= 0.0f ? factor : 0.0f) - ICDistribution[category].first);

		if(difference > 0.0f)
			do
			{
				double min(1.0f);

				for (auto it = ICDistribution.begin(), end = ICDistribution.end(); it != end; ++it)
				{
					if (it->first == category) continue;
					if (!it->second.second && it->second.first != 0.0f)
					{
						sources.push_back(it);
						if (it->second.first < min) min = it->second.first;
					}
				}
				if (sources.size() == 0) return;

				if (min > difference / sources.size()) min = difference / sources.size();
				for (const auto& it : sources)
					it->second.first -= min;

				ICDistribution[category].first += min * sources.size();

			} while (int(ICDistribution[category].first * 100) != int(factor * 100));
		else if (difference != 0.0f)
		{
			for (auto it = ICDistribution.begin(), end = ICDistribution.end(); it != end; ++it)
			{
				if (it->first == category) continue;
				if (!it->second.second)
					sources.push_back(it);
			}
			if (sources.size() == 0) return;

			for (const auto& it : sources)
				it->second.first -= difference / sources.size();

			ICDistribution[category].first += difference;
		}
	}

	void ResourceDistributor::setICDistributionValueLock(const ICDistributionCategory category, const bool lock)
	{
		ICDistributionLock.lock();
		ICDistribution[category].second = lock;
		ICDistributionLock.unlock();
	}
	
	void ResourceDistributor::useResourcesForIC()
	{
		if (IC.first > 0.0f)
		{
			float resourceBottleneck(1.0f);

			auto& energy(resources[Energy]);
			const float reqEnergy(IC.first * ENERGY_PER_IC_POINT * energy[Used].mult);
			if (energy.amount < reqEnergy) resourceBottleneck = energy[Generated].get() / reqEnergy;

			auto& metal(resources[Metal]);
			const float reqMetal(IC.first * METAL_PER_IC_POINT * metal[Used].mult);
			if (metal.amount < reqMetal && metal[Generated].get() / reqMetal < resourceBottleneck)
					resourceBottleneck = metal[Generated].get() / reqMetal;

			auto& rareMats(resources[RareMaterials]);
			const float reqRareMats(IC.first * RARE_MATERIALS_PER_IC_POINT * rareMats[Used].mult);
			if (rareMats.amount < reqRareMats && rareMats[Generated].get() / reqRareMats < resourceBottleneck)
					resourceBottleneck = rareMats[Generated].get() / reqRareMats;

			energy.add(Used, -reqEnergy * resourceBottleneck, true);
			metal.add(Used, -reqMetal * resourceBottleneck, true);
			rareMats.add(Used, -reqRareMats * resourceBottleneck, true);

			ICResourceBottleneck = resourceBottleneck;
		}
	}

	void ResourceDistributor::resourceConversions()
	{
		if (IC.first * IC.second * ICResourceBottleneck > 0.0f)
		{
			{
				auto& energy(resources[Energy]), oil(resources[CrudeOil]);
				const float reqEnergy((1.0f / 20) * IC.first * IC.second * ICResourceBottleneck),
					resourceBottleneck((energy.amount < reqEnergy * energy[ConvertedFrom].mult) ?
						energy.amount / (reqEnergy * energy[ConvertedFrom].mult): 1.0f);
				
				if (energy.amount + energy.get(Total) - resourceBottleneck * reqEnergy > 0.0f)
					energy.add(ConvertedFrom, -resourceBottleneck * reqEnergy, true);
			}

			{
				auto& oil(resources[CrudeOil]), fuel(resources[Fuel]);
				const float reqOil(0.5f * IC.first * IC.second * ICResourceBottleneck),
					resourceBottleneck((oil.amount < reqOil * oil[ConvertedFrom].mult) ?
						oil.amount / (reqOil * oil[ConvertedFrom].mult) : 1.0f);

				if (oil.amount + oil.get(Total) - resourceBottleneck * reqOil > 0.0f)
					oil.add(ConvertedFrom, -resourceBottleneck * reqOil, true);
			}
			resources[CrudeOil].add(ConvertedTo, -resources[Energy][ConvertedFrom].add, true);
			resources[Fuel].add(ConvertedTo, -resources[CrudeOil][ConvertedFrom].add, true);
		}
	}
	
	void ResourceDistributor::distributeIC()
	{
		wastedIC = 0.0f;

		ICDistributionLock.lock();
		wastedIC = wastedIC + Production::get(tag).setIC(IC.first * IC.second * ICResourceBottleneck * ICDistribution[ToProductionLine].first);
		// wastedIC += Upgrades::...
		// wastedIC += ...
		ICDistributionLock.unlock();
	}

	void ResourceDistributor::calculateMoneyChangeAmount()
	{
		resources[Money].add(Generated, (IC.first * IC.second * ICResourceBottleneck - wastedIC) / 20.0f, true);
	}	
}