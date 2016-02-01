#include "Region.h"

#include "ResourceDistributor.h"

#include "Production.h"

namespace bEnd
{
	unordered_map<Tag, unique_ptr<ResourceDistributor>> ResourceDistributor::resourceDistributors;
	const float ResourceDistributor::BASE_IC = 1.0f, ResourceDistributor::ENERGY_PER_IC_POINT = 2.0f, ResourceDistributor::METAL_PER_IC_POINT = 1.0f, ResourceDistributor::RARE_MATERIALS_PER_IC_POINT = 0.5f;


	ResourceDistributor::ResourceDistributor()
	{
		manpower.second.second = 1.0f;

		ICDistribution[ToProductionLine].first = 1.0f;

		for (unsigned char it = 0; it < Resource::Last; it++)
			for (unsigned char it1 = 0; it1 < ResourceChangeCategory::Last; it1++)
				resources[Resource(it)].second[ResourceChangeCategory(it1)].second = 1.0f;
	}

	const bool ResourceDistributor::contains(const std::map<Resource, float>& _rVal)const
	{
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

		manpower.first += manpower.second.first * manpower.second.first;

		for (unsigned char it = 0; it < Resource::Last; it++)
			for (unsigned char it1 = 0; it1 < ResourceChangeCategory::Last; it1++)
			{
				auto& buffer = resources[Resource(it)];
				buffer.first += buffer.second[ResourceChangeCategory(it1)].first * buffer.second[ResourceChangeCategory(it1)].second;
			}

		Production::get(tag).update();
		// Upgrades::...
		// ...
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

	void ResourceDistributor::calculateICResourceBottleneck()
	{
		if (IC.first > 0.0f)
		{
			float resourceBottleneck = 1.0f;

			if (resources[Energy].first < IC.first * ENERGY_PER_IC_POINT * resources[Energy].second[Used].second)
				resourceBottleneck = (resources[Energy].second[Generated].first * resources[Energy].second[Generated].second + resources[Energy].second[Traded].first * resources[Energy].second[Traded].second) / (IC.first * 2.0f * resources[Energy].second[Used].second);

			if (resources[Metal].first < IC.first * METAL_PER_IC_POINT * resources[Metal].second[Used].second)
				if ((resources[Metal].second[Generated].first * resources[Metal].second[Generated].second + resources[Metal].second[Traded].first * resources[Metal].second[Traded].second) / (IC.first * 1.0f * resources[Metal].second[Used].second) < resourceBottleneck)
					resourceBottleneck = (resources[Metal].second[Generated].first * resources[Metal].second[Generated].second + resources[Metal].second[Traded].first * resources[Metal].second[Traded].second) / (IC.first * 1.0f * resources[Metal].second[Used].second);

			if (resources[RareMaterials].first < IC.first * RARE_MATERIALS_PER_IC_POINT * resources[RareMaterials].second[Used].second)
				if ((resources[RareMaterials].second[Generated].first * resources[RareMaterials].second[Generated].second + resources[RareMaterials].second[Traded].first * resources[RareMaterials].second[Traded].second) / (IC.first * 0.5f * resources[RareMaterials].second[Used].second) < resourceBottleneck)
					resourceBottleneck = (resources[RareMaterials].second[Generated].first * resources[RareMaterials].second[Generated].second + resources[RareMaterials].second[Traded].first * resources[RareMaterials].second[Traded].second) / (IC.first * 0.5f * resources[RareMaterials].second[Used].second);

			ICResourceBottleneck = resourceBottleneck;
		}
	}

	void ResourceDistributor::resourceConversions()
	{
		if (IC.first * IC.second * ICResourceBottleneck > 0.0f)
		{
			{
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
			}
		}
	}


	void ResourceDistributor::distributeIC()
	{
		wastedIC = 0.0f;

		wastedIC += Production::get(tag).setIC(IC.first * IC.second * ICDistribution[ToProductionLine].first);
		// wastedIC += Upgrades::...

		resources[Energy].second[Used].first = (IC.first * IC.second * ICResourceBottleneck - wastedIC) * ((-1) * ENERGY_PER_IC_POINT);
		resources[Metal].second[Used].first = (IC.first * IC.second * ICResourceBottleneck - wastedIC) * ((-1) * METAL_PER_IC_POINT);
		resources[RareMaterials].second[Used].first = (IC.first * IC.second * ICResourceBottleneck - wastedIC) * ((-1) * RARE_MATERIALS_PER_IC_POINT);
	}

	void ResourceDistributor::calculateMoneyChangeAmount()
	{
		resources[Money].second[Generated].first = (IC.first * IC.second * ICResourceBottleneck - wastedIC) / 20.0f;		
	}	
}