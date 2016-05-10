#include "Region.h"

#include "ResourceDistributor.h"

#include "Production.h"

#include <iostream>

namespace bEnd
{
	unordered_map<Tag, unique_ptr<ResourceDistributor>> ResourceDistributor::resourceDistributors;
	const float ResourceDistributor::BASE_IC = 1.0f, ResourceDistributor::ENERGY_PER_IC_POINT = 2.0f, ResourceDistributor::METAL_PER_IC_POINT = 1.0f, ResourceDistributor::RARE_MATERIALS_PER_IC_POINT = 0.5f;
	
	ResourceDistributor::ResourceDistributor(const Tag& tag)
		: tag(tag)
	{
		manpower.second = std::make_pair(0.0f, 1.0f);

		for (auto it(0); it <= ICDistributionCategory::ToLendLease; it++)
			ICDistribution[ICDistributionCategory(it)] = std::make_pair(0.0f, false);
		ICDistribution[ToProductionLine] = std::make_pair(1.0f, false);

		for (auto it(0); it < Resource::Last; it++)
		{
			resources[Resource(it)].first = 0;
			for (unsigned char it1 = 0; it1 < ResourceChangeCategory::Last; it1++)
				resources[Resource(it)].second[ResourceChangeCategory(it1)] = std::make_pair(0.0f, 1.0f);
			resources[Resource(it)].second[Total] = std::make_pair(0.0f, 1.0f);
		}
	}

	void ResourceDistributor::loadFromSave(const FileProcessor::Statement& source)
	{
		if (source.lValue != "distribution") return;

		setICDistributionValue(ToUpgrades, std::stof(source.rStrings.at(0)));
		setICDistributionValueLock(ToUpgrades, true);
		setICDistributionValue(ToReinforcement, std::stof(source.rStrings.at(1)));
		setICDistributionValueLock(ToReinforcement, true);
		setICDistributionValue(ToSupplyProduction, std::stof(source.rStrings.at(2)));
		setICDistributionValueLock(ToSupplyProduction, true);
		setICDistributionValue(ToProductionLine, std::stof(source.rStrings.at(3)));
		setICDistributionValueLock(ToProductionLine, true);

		setICDistributionValueLock(ToUpgrades);
		setICDistributionValueLock(ToReinforcement);
		setICDistributionValueLock(ToSupplyProduction);
		setICDistributionValueLock(ToProductionLine);
	}

	const bool ResourceDistributor::contains(const std::map<Resource, float>& _rVal)const
	{
		std::lock_guard<std::mutex> guard(resourcesLock);
		for (auto it = _rVal.begin(), end = _rVal.end(); it != end; ++it)
			if (resources.at(it->first).first < it->second) return false;
		return true;
	}

	void ResourceDistributor::update()
	{
		calculateICResourceBottleneck();
		resourceConversions();
		distributeIC();
		calculateMoneyChangeAmount();

		manpower.first = manpower.first + (manpower.second.first * manpower.second.first) / 30;

		resourcesLock.lock();
		for (auto it(0); it < Resource::Last; it++)
		{
			auto& buffer(resources[Resource(it)]);
			buffer.second[Total].first = 0;
			for (auto it1(0); it1 < ResourceChangeCategory::Last; it1++)
				buffer.second[Total].first += buffer.second[ResourceChangeCategory(it1)].first * buffer.second[ResourceChangeCategory(it1)].second;
			buffer.first += buffer.second[Total].first;
		}
		resourcesLock.unlock();

		Production::get(tag).update();
		// Upgrades::...
		// ...
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
	
	void ResourceDistributor::calculateICResourceBottleneck()
	{
		if (IC.first > 0.0f)
		{
			float resourceBottleneck = 1.0f;

			resourcesLock.lock();
			if (resources[Energy].first < IC.first * ENERGY_PER_IC_POINT * resources[Energy].second[Used].second)
				resourceBottleneck = (resources[Energy].second[Generated].first * resources[Energy].second[Generated].second + resources[Energy].second[Traded].first * resources[Energy].second[Traded].second) / (IC.first * 2.0f * resources[Energy].second[Used].second);

			if (resources[Metal].first < IC.first * METAL_PER_IC_POINT * resources[Metal].second[Used].second)
				if ((resources[Metal].second[Generated].first * resources[Metal].second[Generated].second + resources[Metal].second[Traded].first * resources[Metal].second[Traded].second) / (IC.first * 1.0f * resources[Metal].second[Used].second) < resourceBottleneck)
					resourceBottleneck = (resources[Metal].second[Generated].first * resources[Metal].second[Generated].second + resources[Metal].second[Traded].first * resources[Metal].second[Traded].second) / (IC.first * 1.0f * resources[Metal].second[Used].second);

			if (resources[RareMaterials].first < IC.first * RARE_MATERIALS_PER_IC_POINT * resources[RareMaterials].second[Used].second)
				if ((resources[RareMaterials].second[Generated].first * resources[RareMaterials].second[Generated].second + resources[RareMaterials].second[Traded].first * resources[RareMaterials].second[Traded].second) / (IC.first * 0.5f * resources[RareMaterials].second[Used].second) < resourceBottleneck)
					resourceBottleneck = (resources[RareMaterials].second[Generated].first * resources[RareMaterials].second[Generated].second + resources[RareMaterials].second[Traded].first * resources[RareMaterials].second[Traded].second) / (IC.first * 0.5f * resources[RareMaterials].second[Used].second);
			resourcesLock.unlock();

			ICResourceBottleneck = resourceBottleneck;
		}
	}

	void ResourceDistributor::resourceConversions()
	{
		if (IC.first * IC.second * ICResourceBottleneck > 0.0f)
		{
			{
				resourcesLock.lock();
				float resourceBottleneck =
					resources[Energy].first < 0.5f * IC.first * IC.second * ICResourceBottleneck * resources[Energy].second[ConvertedFrom].second ?
					resources[Energy].first / (0.5f * IC.first * IC.second * ICResourceBottleneck * resources[Energy].second[ConvertedFrom].second)
					: 1.0f;

				resources[Energy].second[ConvertedFrom].first = resourceBottleneck * (-0.5f) * IC.first * IC.second * ICResourceBottleneck;
				resources[Energy].first += resources[Energy].second[ConvertedFrom].first * resources[Energy].second[ConvertedFrom].second;

				resources[CrudeOil].second[ConvertedTo].first = resourceBottleneck * IC.first * IC.second * ICResourceBottleneck / 20.0f;
				resources[CrudeOil].first += resources[CrudeOil].second[ConvertedTo].first * resources[CrudeOil].second[ConvertedTo].second;
			}

			{
				float resourceBottleneck = resources[CrudeOil].first < 0.5f * IC.first * IC.second * ICResourceBottleneck * resources[CrudeOil].second[ConvertedFrom].second ?
					resources[CrudeOil].first / (0.5f * IC.first * IC.second * ICResourceBottleneck * resources[CrudeOil].second[ConvertedFrom].second)
					: 1.0f;

				resources[CrudeOil].second[ConvertedFrom].first = resourceBottleneck * (-0.5f) * IC.first * IC.second * ICResourceBottleneck;
				resources[CrudeOil].first += resources[CrudeOil].second[ConvertedFrom].first * resources[CrudeOil].second[ConvertedFrom].second;

				resources[Fuel].second[ConvertedTo].first = resourceBottleneck * 0.5f * IC.first * IC.second * ICResourceBottleneck;
				resources[Fuel].first += resources[Fuel].second[ConvertedTo].first * resources[Fuel].second[ConvertedTo].second;
				resourcesLock.unlock();
			}
		}
	}
	
	void ResourceDistributor::distributeIC()
	{
		wastedIC = 0.0f;

		ICDistributionLock.lock();
		wastedIC = wastedIC + Production::get(tag).setIC(IC.first * IC.second * ICDistribution[ToProductionLine].first);
		// wastedIC += Upgrades::...
		// wastedIC += ...
		ICDistributionLock.unlock();

		resourcesLock.lock();
		resources[Energy].second[Used].first = (IC.first * IC.second * ICResourceBottleneck - wastedIC) * ((-1) * ENERGY_PER_IC_POINT);
		resources[Metal].second[Used].first = (IC.first * IC.second * ICResourceBottleneck - wastedIC) * ((-1) * METAL_PER_IC_POINT);
		resources[RareMaterials].second[Used].first = (IC.first * IC.second * ICResourceBottleneck - wastedIC) * ((-1) * RARE_MATERIALS_PER_IC_POINT);
		resourcesLock.unlock();
	}

	void ResourceDistributor::calculateMoneyChangeAmount()
	{
		resourcesLock.lock();
		resources[Money].second[Generated].first = (IC.first * IC.second * ICResourceBottleneck - wastedIC) / 20.0f;
		resourcesLock.unlock();
	}	
}