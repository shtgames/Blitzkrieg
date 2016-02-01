#ifndef RESOURCE_DISTRIBUTOR_BACKEND
#define RESOURCE_DISTRIBUTOR_BACKEND

#include "Resources.h"
#include "Tag.h"

#include <map>
#include <unordered_map>
#include <memory>

using namespace std;

namespace bEnd
{
	class Region;
	class ResourceDistributor final
	{
	public:
		enum ICDistributionCategory
		{
			ToUpgrades,
			ToReinforcement,
			ToSupplyProduction,
			ToProductionLine,
			ToConsumerGoods,
			ToLendLease
		};

		enum ResourceChangeCategory
		{
			Generated,
			Used,
			Traded,
			ConvertedTo,
			ConvertedFrom,
			IntoNetwork,
			ReturnedToStockpile,
			Last
		};

		~ResourceDistributor() = default;

		const float getDistributedICAmount(const ICDistributionCategory category)const { return ICDistribution[category].first * IC.first * IC.second * ICResourceBottleneck; }
		void setICDistributionValue(const ICDistributionCategory category, const double factor);
		void setICDistributionValueLock(const ICDistributionCategory category, const bool lock = false);

		const float getICAmount()const { return IC.first * IC.second; }
		const float getResourceGain(const Resource resource, const ResourceChangeCategory category)const { return resources[resource].second[category].first * resources[resource].second[category].second; };
		const float getResourceAmount(const Resource resource)const { return resources[resource].first; };
		const float getManpowerGain()const { return manpower.second.first * manpower.second.second; }
		const float getManpowerAmount()const { return manpower.first; }
		
		void changeICAmount(const float amount) { IC.first += amount; }
		void changeManpowerGain(const float amount) { manpower.second.first += amount; }
		void changeManpowerAmount(const float amount) { manpower.first + amount >= 0.0f ? manpower.first += amount : manpower.first = 0.0f; }
		void changeResourceGain(const Resource resource, const float changeAmount, const ResourceChangeCategory category = Generated) { resources[resource].second[category].first += changeAmount; }
		void changeResourceAmount(const Resource resource, const float amount) { resources[resource].first + amount >= 0.0f ? resources[resource].first += amount : resources[resource].first = 0.0f; }
		
		const bool contains(const map<Resource, float>& resources)const;
		void update();

		static const bool loadFromFile(ifstream&);
		static const bool exists(const Tag& tag) { if (resourceDistributors.count(tag) && resourceDistributors.at(tag)) return true; return false; }
		static ResourceDistributor& get(const Tag& tag) { return *resourceDistributors.at(tag); }

	private:

		typedef map<ResourceChangeCategory, pair<float, float>>(ResourceChangeSummary);

		ResourceDistributor(const ResourceDistributor&) = default;
		ResourceDistributor(ResourceDistributor&&) = default;
		ResourceDistributor();

		void calculateICResourceBottleneck();
		void resourceConversions();
		void distributeIC();
		void calculateMoneyChangeAmount();

		float                                                     wastedIC = 0.0f, ICResourceBottleneck = 1.0f;
		pair<float, float>                                        IC = make_pair(BASE_IC, 1.0f);
		pair<float, pair<float, float>>                           manpower;
		mutable map<Resource, pair<float, ResourceChangeSummary>> resources;

		mutable map<ICDistributionCategory, std::pair<float, bool>> ICDistribution;

		const Tag tag;


		static unordered_map<Tag, unique_ptr<ResourceDistributor>> resourceDistributors;
		static const float BASE_IC, ENERGY_PER_IC_POINT, METAL_PER_IC_POINT, RARE_MATERIALS_PER_IC_POINT;
	};
}

#endif